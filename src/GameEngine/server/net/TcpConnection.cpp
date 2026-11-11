/**
 * @file TcpConnection.cpp
 * @brief tcp connection implementation
 *
 * @author Seb
 * @date 2026-10-06
**/

#include "GameEngine/server/net/TcpConnection.hpp"
#include "GameEngine/server/lobby/RoomManager.hpp"
#include "GameEngine/protocol/TcpProtocol.hpp"
#include <array>
#include <algorithm>
#include <iostream>
#include <utility>
#include <cctype>

namespace {

constexpr std::size_t kMaxWriteQueue = 128;

const char* typeName(server::Type type)
{
    switch (type) {
        case server::Type::Connect: return "CONNECT";
        case server::Type::ConnectOk: return "CONNECT_OK";
        case server::Type::ListRooms: return "LIST_ROOMS";
        case server::Type::RoomList: return "ROOM_LIST";
        case server::Type::Spectate: return "SPECTATE";
        case server::Type::Spectating: return "SPECTATING";
        case server::Type::Leave: return "LEAVE";
        case server::Type::Left: return "LEFT";
        case server::Type::Error: return "ERROR";
    }
    return "UNKNOWN";
}

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

    asio::error_code endpointError;
    const auto endpoint = this->_socket.remote_endpoint(endpointError);
    if (!endpointError)
        std::cerr << "[TCP] connected peer=" << endpoint << '\n';

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
    std::cerr << "[TCP] sent type=" << typeName(static_cast<server::Type>(message.front()))
        << " bytes=" << message.size();
    if (!this->_username.empty())
        std::cerr << " user=" << this->_username;
    std::cerr << '\n';
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

    asio::error_code endpointError;
    const auto endpoint = this->_socket.remote_endpoint(endpointError);
    std::cerr << "[TCP] disconnected user=" << (this->_username.empty() ? "<unauthenticated>" : this->_username);
    if (!endpointError)
        std::cerr << " peer=" << endpoint;
    std::cerr << '\n';

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

void server::TcpConnection::left(LeaveReason reason)
{
    if (!this->_token)
        return;
    this->_token.reset();
    this->_state = State::Lobby;
    this->send(Buffer{static_cast<Byte>(server::Type::Left), static_cast<Byte>(reason)});
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
    asio::error_code endpointError;
    const auto endpoint = this->_socket.remote_endpoint(endpointError);
    std::cerr << "[TCP] received type=" << typeName(type) << " bytes=" << frame.size();
    if (!this->_username.empty())
        std::cerr << " user=" << this->_username;
    if (!endpointError)
        std::cerr << " peer=" << endpoint;
    std::cerr << '\n';
    Buffer payload(frame.begin() + 1, frame.end());
    std::size_t offset = 0;

    auto badRequest = [this] {
        this->_closeAfterWrite = true;
        this->send(Buffer{static_cast<Byte>(server::Type::Error), static_cast<Byte>(LobbyError::BadRequest)});
    };
    auto sendError = [this](LobbyError error) {
        std::cerr << "[TCP] lobby error user=" << this->_username << " code=" << static_cast<unsigned>(error) << '\n';
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
        if (version != ProtocolVersion || !validUsername(name)) {
            this->_closeAfterWrite = true;
            sendError(version != ProtocolVersion ? LobbyError::BadVersion : LobbyError::BadName);
            return;
        }
        this->_username = std::move(name);
        this->_state = State::Lobby;
        std::cerr << "[TCP] authenticated user=" << this->_username << '\n';
        asio::error_code ignored;
        this->_deadline.cancel(ignored);
        this->send(Buffer{static_cast<Byte>(server::Type::ConnectOk)});
        return;
    }

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
            response.push_back(room.spectators);
            response.push_back(room.maxSpectators);
        }
        this->send(response);
        return;
    }
    if (type == server::Type::Leave && payload.empty()) {
        if (this->_token) {
            const SessionToken token = *this->_token;
            this->_token.reset();
            this->_state = State::Lobby;
            this->_rooms.leave(token, LeaveReason::Requested);
        }
        return;
    }
    if (type == server::Type::Spectate && this->_state == State::Lobby) {
        std::uint32_t id = 0;
        if (!TcpProtocol::readU32(payload, offset, id) || offset != payload.size()) {
            badRequest();
            return;
        }
        auto result = this->_rooms.spectate(this->shared_from_this(), id);
        if (!result) {
            sendError(result.error());
            return;
        }
        this->_state = State::Spectating;
        Buffer response{static_cast<Byte>(server::Type::Spectating)};
        TcpProtocol::appendU32(response, result->room);
        TcpProtocol::appendU64(response, result->token);
        TcpProtocol::appendU16(response, this->_conf.udpPort);
        this->send(response);
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
