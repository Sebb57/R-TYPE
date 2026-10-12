/**
 * @file TcpConnection.cpp
 * @brief tcp connection implementation
 *
 * @author Seb
 * @date 2026-10-06
**/

#include "server/net/TcpConnection.hpp"
#include "server/lobby/RoomManager.hpp"
#include "protocol/TcpProtocol.hpp"
#include <array>
#include <algorithm>
#include <utility>
#include <cctype>

namespace {

constexpr std::size_t kMaxWriteQueue = 128;

bool validUsername(const std::string& name)
{
    return !name.empty() && name.size() <= 16 && std::ranges::all_of(name, [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-';
    });
}

} // namespace

server::TcpConnection::TcpConnection(asio::ip::tcp::socket socket, RoomManager& rooms, const ServerConfig& conf)
    : _socket(std::move(socket))
    , _deadline(_socket.get_executor())
    , _rooms(rooms)
    , _conf(conf)
{}

void server::TcpConnection::start()
{
    if (!this->_socket.is_open())
        return;

    this->_deadline.expires_after(this->_conf.handshakeTimeout);
    auto self = this->shared_from_this();
    this->_deadline.async_wait([self](const asio::error_code& error) {
        if (!error && self->_state == State::AwaitingConnect)
            self->close();
    });
    this->readFrameLength();
}

void server::TcpConnection::send(const Buffer& message)
{
    if (!this->_socket.is_open() || message.empty() || message.size() > MaxFrameSize)
        return;
    if (this->_writeQueue.size() >= kMaxWriteQueue) {
        this->close();
        return;
    }

    Buffer frame;
    frame.reserve(message.size() + 2);
    TcpProtocol::appendU16(frame, static_cast<std::uint16_t>(message.size()));
    frame.insert(frame.end(), message.begin(), message.end());

    const bool idle = this->_writeQueue.empty();
    this->_writeQueue.push_back(std::move(frame));
    if (idle)
        this->flushWrites();
}

void server::TcpConnection::close()
{
    if (!this->_socket.is_open())
        return;

    asio::error_code ignored;
    this->_deadline.cancel(ignored);
    this->_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    this->_socket.close(ignored);
    this->_writeQueue.clear();
    if (this->_token) {
        this->_rooms.leave(*this->_token, LeaveReason::ConnectionLost);
        this->_token.reset();
    }
}

void server::TcpConnection::readFrameLength()
{
    if (!this->_socket.is_open())
        return;

    auto self = this->shared_from_this();
    auto header = std::make_shared<std::array<Byte, 2>>();
    asio::async_read(this->_socket, asio::buffer(*header), [self, header](const asio::error_code& error, std::size_t) {
        if (error) { 
            self->close(); 
            return;
        }
        const auto length = static_cast<std::uint16_t>((*header)[0] | (static_cast<std::uint16_t>((*header)[1]) << 8));
        if (length == 0 || length > MaxFrameSize) {
            self->close();  
            return;
        }
        self->readFrameBody(length);
    });
}

void server::TcpConnection::readFrameBody(std::uint16_t length)
{
    if (!this->_socket.is_open() || length == 0 || length > MaxFrameSize)
        return;

    auto self = this->shared_from_this();
    auto frame = std::make_shared<Buffer>(length);
    asio::async_read(this->_socket, asio::buffer(*frame), [self, frame](const asio::error_code& error, std::size_t) {
        if (error) {
            self->close();
            return;
        }
        self->onFrame(*frame);
        if (self->_socket.is_open()) self->readFrameLength();
    });
}

void server::TcpConnection::onFrame(const Buffer& frame)
{
    if (frame.empty()) {
        this->close();
        return;
    }

    const auto type = static_cast<server::Type>(frame.front());
    Buffer payload(frame.begin() + 1, frame.end());
    std::size_t offset = 0;

    auto badRequest = [this] {
        this->_closeAfterWrite = true;
        this->send(Buffer{static_cast<Byte>(server::Type::Error), 10});
    };
    auto sendError = [this](LobbyError error) {
        this->send(Buffer{static_cast<Byte>(server::Type::Error), static_cast<uint8_t>(error)});
    };

    if (this->_state == State::AwaitingConnect) {
        std::uint8_t version = 0;
        std::string name;
        if (type != server::Type::Connect || !TcpProtocol::readU8(payload, offset, version) ||
            !TcpProtocol::readString(payload, offset, 16, name) || offset != payload.size()) {
            badRequest();
            return;
        }
        if (version != 1 || !validUsername(name)) {
            this->_closeAfterWrite = true;
            const Byte code = version != 1 ? 1 : 2;
            this->send(Buffer{static_cast<Byte>(server::Type::Error), code});
            return;
        }
        this->_username = std::move(name);
        this->_state = State::Lobby;
        asio::error_code ignored;
        this->_deadline.cancel(ignored);
        this->send(Buffer{static_cast<Byte>(server::Type::ConnectOk)});
        return;
    }

    if (this->_state == State::Lobby) {
        if (type == server::Type::ListRooms && payload.empty()) {
            const auto rooms = this->_rooms.list();
            if (rooms.size() > 255) {
                badRequest();
                return;
            }
            Buffer response{static_cast<Byte>(server::Type::RoomList), static_cast<Byte>(rooms.size())};
            for (const auto& room : rooms) {
                TcpProtocol::appendU32(response, room.id);
                TcpProtocol::appendString(response, room.name);
                response.push_back(room.players);
                response.push_back(room.maxPlayers);
                response.push_back(static_cast<Byte>(room.state));
            }
            this->send(response);
            return;
        }
        if (type == server::Type::CreateRoom) {
            std::string name;
            std::uint8_t capacity = 0;
            if (!TcpProtocol::readString(payload, offset, 32, name) || !TcpProtocol::readU8(payload, offset, capacity) || offset != payload.size()) {
                badRequest();
                return;
            }
            auto result = this->_rooms.create(this->shared_from_this(), std::move(name), capacity);
            if (!result) {
                sendError(result.error());
                return;
            }
            this->_token = result->token;
            this->_state = State::InRoom;
            Buffer response{static_cast<Byte>(server::Type::RoomJoined)};
            TcpProtocol::appendU32(response, result->room);
            TcpProtocol::appendU64(response, result->token);
            response.push_back(result->slot);
            TcpProtocol::appendU16(response, this->_conf.udpPort);
            this->send(response);
            this->_rooms.refreshRoom(result->room);
            return;
        }
        if (type == server::Type::JoinRoom) {
            std::uint32_t id = 0;
            if (!TcpProtocol::readU32(payload, offset, id) || offset != payload.size()) {
                badRequest();
                return;
            }
            auto result = this->_rooms.join(this->shared_from_this(), id);
            if (!result) {
                sendError(result.error());
                return;
            }
            this->_token = result->token;
            this->_state = State::InRoom;
            Buffer response{static_cast<Byte>(server::Type::RoomJoined)};
            TcpProtocol::appendU32(response, result->room);
            TcpProtocol::appendU64(response, result->token);
            response.push_back(result->slot);
            TcpProtocol::appendU16(response, this->_conf.udpPort);
            this->send(response);
            this->_rooms.refreshRoom(result->room);
            return;
        }
        badRequest();
        return;
    }

    if (type == server::Type::LeaveRoom && payload.empty() && this->_token) {
        this->_rooms.leave(*this->_token, LeaveReason::Requested);
        this->_token.reset();
        this->_state = State::Lobby;
        return;
    }
    if (type == server::Type::StartGame && payload.empty() && this->_token) {
        auto result = this->_rooms.startGame(*this->_token);
        if (!result) { sendError(result.error()); return; }
        this->send(Buffer{static_cast<Byte>(server::Type::GameStarted)});
        return;
    }
    badRequest();
}

void server::TcpConnection::flushWrites()
{
    if (!this->_socket.is_open() || this->_writeQueue.empty())
        return;

    auto self = this->shared_from_this();
    asio::async_write(this->_socket, asio::buffer(this->_writeQueue.front()), [self](const asio::error_code& error, std::size_t) {
        if (error) {
            self->close();
            return;
        }
        self->_writeQueue.pop_front();
        if (!self->_writeQueue.empty())
            self->flushWrites();
        else if 
            (self->_closeAfterWrite) self->close();
    });
}
