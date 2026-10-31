/**
 * @file NetworkClient.cpp
 * @brief client network layer implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/network/NetworkClient.hpp"
#include <chrono>
#include <filesystem>
#include <iterator>
#include <utility>
#include <vector>

namespace {

const std::array<std::string, 6> Errors{
    "bad request", "incompatible version", "invalid player name", "game not found", "too many spectators", "too many games",
};
const std::array<std::string, 4> LeaveReasons{
    "Stopped spectating", "Connection lost", "Spectating timed out", "The game ended",
};

template <std::size_t Size>
std::string describe(const std::array<std::string, Size>& names, std::uint8_t code)
{
    return code < names.size() ? names[code] : "error " + std::to_string(code);
}

std::string localImage(const std::string& image)
{
    const std::filesystem::path path(image);
    if (path.has_root_path())
        return {};
    for (const auto& part : path)
        if (part == "..")
            return {};
    return image;
}

} // namespace

client::NetworkClient::NetworkClient()
    : _tcpHandlers{
        {server::Type::ConnectOk, &NetworkClient::onConnectOk},
        {server::Type::RoomList, &NetworkClient::onRoomList},
        {server::Type::Spectating, &NetworkClient::onSpectating},
        {server::Type::Left, &NetworkClient::onLeft},
        {server::Type::Error, &NetworkClient::onError},
    }
    , _udpHandlers{
        {server::UdpType::HelloAck, &NetworkClient::onHelloAck},
        {server::UdpType::Snapshot, &NetworkClient::onSnapshot},
    }
    , _work(asio::make_work_guard(this->_io))
    , _tcp(this->_io)
    , _udp(this->_io)
    , _helloTimer(this->_io)
    , _pingTimer(this->_io)
    , _thread([this] { this->_io.run(); })
{}

client::NetworkClient::~NetworkClient()
{
    this->_io.stop();
}

client::ClientView client::NetworkClient::view() const
{
    std::scoped_lock lock(this->_mutex);
    return this->_view;
}

void client::NetworkClient::connect(std::string host, std::uint16_t port, std::string username)
{
    asio::post(this->_io, [this, host = std::move(host), port, username = std::move(username)] {
        this->setStatus("Connecting to " + host + "...");
        asio::error_code error;
        const auto address = asio::ip::make_address_v4(host, error);
        if (error) {
            this->setStatus("Invalid IPv4 address " + host);
            return;
        }
        this->_hostAddress = address;
        this->_username = username;
        this->_tcp.close(error);
        this->_tcp = asio::ip::tcp::socket(this->_io);
        this->_tcp.async_connect({address, port}, [this](const asio::error_code& ec) {
            if (ec) {
                this->setStatus("Connection failed: " + ec.message());
                return;
            }
            server::Buffer payload{server::ProtocolVersion};
            server::TcpProtocol::appendString(payload, this->_username);
            this->sendTcp(server::TcpProtocol::message(server::Type::Connect, std::move(payload)));
            this->readLength();
        });
    });
}

void client::NetworkClient::disconnect()
{
    asio::post(this->_io, [this] { this->onDisconnect("left the server"); });
}

void client::NetworkClient::listRooms()
{
    this->sendTcp(server::TcpProtocol::message(server::Type::ListRooms));
}

void client::NetworkClient::spectate(std::uint32_t id)
{
    server::Buffer payload;
    server::TcpProtocol::appendU32(payload, id);
    this->sendTcp(server::TcpProtocol::message(server::Type::Spectate, std::move(payload)));
}

void client::NetworkClient::leave()
{
    asio::post(this->_io, [this] {
        this->stopUdp();
        std::scoped_lock lock(this->_mutex);
        this->_view.spectating = false;
        this->_view.udpReady = false;
        this->_view.entities.clear();
        this->_view.status = LeaveReasons[static_cast<std::size_t>(server::LeaveReason::Requested)];
    });
    this->sendTcp(server::TcpProtocol::message(server::Type::Leave));
}

void client::NetworkClient::setStatus(std::string status)
{
    std::scoped_lock lock(this->_mutex);
    this->_view.status = std::move(status);
}

void client::NetworkClient::sendTcp(server::Buffer message)
{
    asio::post(this->_io, [this, message = std::move(message)]() mutable {
        if (!this->_tcp.is_open())
            return;
        auto frame = std::make_shared<server::Buffer>(server::TcpProtocol::frame(message));
        if (frame->empty())
            return;
        const bool idle = this->_writes.empty();
        this->_writes.push_back(std::move(frame));
        if (idle)
            this->writeNext();
    });
}

void client::NetworkClient::writeNext()
{
    if (this->_writes.empty() || !this->_tcp.is_open())
        return;
    auto frame = this->_writes.front();
    asio::async_write(this->_tcp, asio::buffer(*frame), [this, frame](const asio::error_code& error, std::size_t) {
        if (error) {
            if (error != asio::error::operation_aborted)
                this->onDisconnect(error.message());
            return;
        }
        if (this->_writes.empty() || this->_writes.front() != frame)
            return;
        this->_writes.pop_front();
        if (!this->_writes.empty())
            this->writeNext();
    });
}

void client::NetworkClient::readLength()
{
    auto header = std::make_shared<std::array<server::Byte, 2>>();
    asio::async_read(this->_tcp, asio::buffer(*header), [this, header](const asio::error_code& error, std::size_t) {
        if (error) {
            if (error != asio::error::operation_aborted)
                this->onDisconnect(error.message());
            return;
        }
        const auto size = static_cast<std::uint16_t>((*header)[0] | ((*header)[1] << 8));
        if (size == 0 || size > server::MaxFrameSize) {
            this->onDisconnect("Invalid server frame");
            return;
        }
        this->readBody(size);
    });
}

void client::NetworkClient::readBody(std::uint16_t size)
{
    auto body = std::make_shared<server::Buffer>(size);
    asio::async_read(this->_tcp, asio::buffer(*body), [this, body](const asio::error_code& error, std::size_t) {
        if (error) {
            if (error != asio::error::operation_aborted)
                this->onDisconnect(error.message());
            return;
        }
        this->onTcpMessage(*body);
        this->readLength();
    });
}

void client::NetworkClient::onTcpMessage(const server::Buffer& message)
{
    if (message.empty())
        return;
    const auto handler = this->_tcpHandlers.find(static_cast<server::Type>(message[0]));
    if (handler != this->_tcpHandlers.end())
        (this->*handler->second)(message, 1);
}

void client::NetworkClient::onConnectOk(const server::Buffer& /* buffer */, std::size_t /* size */)
{
    {
        std::scoped_lock lock(this->_mutex);
        this->_view.connected = true;
        this->_view.username = this->_username;
        this->_view.status = "Connected as " + this->_username;
    }
    this->listRooms();
}

void client::NetworkClient::onRoomList(const server::Buffer& message, std::size_t offset)
{
    std::uint8_t count = 0;
    if (!server::TcpProtocol::readU8(message, offset, count))
        return;
    std::vector<RoomView> rooms;
    for (std::uint8_t i = 0; i < count; ++i) {
        RoomView room;
        if (!server::TcpProtocol::readU32(message, offset, room.id) ||
            !server::TcpProtocol::readString(message, offset, 32, room.name) ||
            !server::TcpProtocol::readU8(message, offset, room.spectators) ||
            !server::TcpProtocol::readU8(message, offset, room.capacity))
            return;
        rooms.push_back(std::move(room));
    }
    std::scoped_lock lock(this->_mutex);
    this->_view.rooms = std::move(rooms);
}

void client::NetworkClient::onSpectating(const server::Buffer& message, std::size_t offset)
{
    std::uint32_t id = 0;
    std::uint64_t token = 0;
    std::uint16_t port = 0;
    if (!server::TcpProtocol::readU32(message, offset, id) ||
        !server::TcpProtocol::readU64(message, offset, token) ||
        !server::TcpProtocol::readU16(message, offset, port))
        return;
    {
        std::scoped_lock lock(this->_mutex);
        this->_view.spectating = true;
        this->_view.udpReady = false;
        this->_view.room = id;
        this->_view.token = token;
        this->_view.udpPort = port;
        this->_view.tick = 0;
        this->_view.entities.clear();
        this->_view.status = "Joining game " + std::to_string(id) + "...";
    }
    this->startUdp(port, token);
}

void client::NetworkClient::onLeft(const server::Buffer& message, std::size_t offset)
{
    std::uint8_t reason = 0;
    if (!server::TcpProtocol::readU8(message, offset, reason))
        return;
    {
        std::scoped_lock lock(this->_mutex);
        if (!this->_view.spectating)
            return;
        this->_view.spectating = false;
        this->_view.udpReady = false;
        this->_view.entities.clear();
        this->_view.status = describe(LeaveReasons, reason);
    }
    this->stopUdp();
}

void client::NetworkClient::onError(const server::Buffer& message, std::size_t offset)
{
    std::uint8_t code = 0;
    if (server::TcpProtocol::readU8(message, offset, code))
        this->setStatus("Server error: " + describe(Errors, code));
}

void client::NetworkClient::startUdp(std::uint16_t port, std::uint64_t token)
{
    asio::error_code error;
    this->stopUdp();
    this->_udp.open(this->_hostAddress.is_v4() ? asio::ip::udp::v4() : asio::ip::udp::v6(), error);
    if (error) {
        this->setStatus("UDP setup failed: " + error.message());
        return;
    }
    this->_udpEndpoint = {this->_hostAddress, port};
    this->_udpSequence = 0;
    this->sendHello(token);
    this->readUdp();
}

void client::NetworkClient::stopUdp()
{
    asio::error_code ignored;
    this->_helloTimer.cancel(ignored);
    this->_pingTimer.cancel(ignored);
    this->_udp.close(ignored);
    this->_pending = {};
}

void client::NetworkClient::sendHello(std::uint64_t token)
{
    auto bytes = std::make_shared<server::Buffer>(server::UdpProtocol::encode(server::UdpType::Hello, 0, token));
    this->_udp.async_send_to(asio::buffer(*bytes), this->_udpEndpoint, [bytes](const asio::error_code&, std::size_t) {});
    this->_helloTimer.expires_after(std::chrono::seconds(1));
    this->_helloTimer.async_wait([this, token](const asio::error_code& error) {
        if (error || this->_view.udpReady)
            return;
        this->sendHello(token);
    });
}

void client::NetworkClient::readUdp()
{
    this->_udp.async_receive_from(asio::buffer(this->_udpBuffer), this->_udpSender, [this](const asio::error_code& error, std::size_t size) {
        if (error)
            return;
        this->onUdpPacket(server::Buffer(this->_udpBuffer.begin(), this->_udpBuffer.begin() + static_cast<std::ptrdiff_t>(size)));
        this->readUdp();
    });
}

void client::NetworkClient::onUdpPacket(const server::Buffer& packet)
{
    const auto header = server::UdpProtocol::parseHeader(packet, packet.size());
    if (!header)
        return;
    const auto handler = this->_udpHandlers.find(header->type);
    if (handler != this->_udpHandlers.end())
        (this->*handler->second)(packet);
}

void client::NetworkClient::onHelloAck(const server::Buffer& packet)
{
    {
        std::scoped_lock lock(this->_mutex);
        if (packet.size() >= server::UdpHeaderSize + 2 && packet[server::UdpHeaderSize + 1] != 0)
            this->_view.snapshotRate = packet[server::UdpHeaderSize + 1];
        this->_view.udpReady = true;
        this->_view.status = "Waiting for the game...";
    }
    this->beginPingLoop();
}

void client::NetworkClient::onSnapshot(const server::Buffer& packet)
{
    std::size_t offset = server::UdpHeaderSize;
    std::uint32_t tick = 0;
    std::uint8_t part = 0;
    std::uint8_t parts = 0;
    std::uint8_t imageCount = 0;
    if (!server::TcpProtocol::readU32(packet, offset, tick) ||
        !server::TcpProtocol::readU8(packet, offset, part) ||
        !server::TcpProtocol::readU8(packet, offset, parts) ||
        !server::TcpProtocol::readU8(packet, offset, imageCount) || part >= parts || tick <= this->_view.tick)
        return;
    std::vector<std::string> images(imageCount);
    for (std::string& image : images)
        if (!server::TcpProtocol::readString(packet, offset, server::MaxImagePath, image))
            return;
    std::uint8_t count = 0;
    if (!server::TcpProtocol::readU8(packet, offset, count) || packet.size() - offset != count * server::SnapshotEntitySize)
        return;
    std::vector<EntityView> entities(count);
    for (EntityView& entity : entities) {
        std::uint16_t x = 0;
        std::uint16_t y = 0;
        std::uint16_t rotation = 0;
        std::uint16_t width = 0;
        std::uint16_t height = 0;
        std::uint8_t image = server::NoImage;
        server::TcpProtocol::readU32(packet, offset, entity.id);
        server::TcpProtocol::readU16(packet, offset, x);
        server::TcpProtocol::readU16(packet, offset, y);
        server::TcpProtocol::readU16(packet, offset, rotation);
        server::TcpProtocol::readU16(packet, offset, width);
        server::TcpProtocol::readU16(packet, offset, height);
        server::TcpProtocol::readU8(packet, offset, entity.color.r);
        server::TcpProtocol::readU8(packet, offset, entity.color.g);
        server::TcpProtocol::readU8(packet, offset, entity.color.b);
        server::TcpProtocol::readU8(packet, offset, entity.color.a);
        server::TcpProtocol::readU8(packet, offset, image);
        entity.position = Vector2(static_cast<std::int16_t>(x), static_cast<std::int16_t>(y));
        entity.rotation = static_cast<std::int16_t>(rotation);
        entity.size = Vector2(width, height);
        entity.image = image < images.size() ? localImage(images[image]) : "";
    }
    this->assemble(tick, part, parts, std::move(entities));
}

void client::NetworkClient::assemble(std::uint32_t tick, std::uint8_t part, std::uint8_t parts, std::vector<EntityView> entities)
{
    if (tick != this->_pending.tick || parts != this->_pending.parts) {
        if (tick < this->_pending.tick)
            return;
        this->_pending = PendingSnapshot{tick, parts, {}};
    }
    this->_pending.received.try_emplace(part, std::move(entities));
    if (this->_pending.received.size() < parts)
        return;
    std::vector<EntityView> snapshot;
    for (auto& [index, received] : this->_pending.received)
        snapshot.insert(snapshot.end(), std::make_move_iterator(received.begin()), std::make_move_iterator(received.end()));
    this->_pending = {};
    std::scoped_lock lock(this->_mutex);
    this->_view.tick = tick;
    this->_view.entities = std::move(snapshot);
}

void client::NetworkClient::beginPingLoop()
{
    this->_pingTimer.expires_after(std::chrono::seconds(1));
    this->_pingTimer.async_wait([this](const asio::error_code& error) {
        if (error)
            return;
        this->sendPing();
        this->beginPingLoop();
    });
}

void client::NetworkClient::sendPing()
{
    server::Buffer payload;
    server::UdpProtocol::appendU32(payload, ++this->_udpSequence);
    auto bytes = std::make_shared<server::Buffer>(server::UdpProtocol::encode(server::UdpType::Ping, this->_udpSequence, this->_view.token, payload));
    this->_udp.async_send_to(asio::buffer(*bytes), this->_udpEndpoint, [bytes](const asio::error_code&, std::size_t) {});
}

void client::NetworkClient::onDisconnect(const std::string& reason)
{
    asio::error_code ignored;
    this->_tcp.close(ignored);
    this->stopUdp();
    this->_writes.clear();
    std::scoped_lock lock(this->_mutex);
    this->_view.connected = false;
    this->_view.spectating = false;
    this->_view.udpReady = false;
    this->_view.rooms.clear();
    this->_view.entities.clear();
    this->_view.status = "Disconnected: " + reason;
}
