/**
 * @file NetworkClient.cpp
 * @brief client network layer implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/network/NetworkClient.hpp"
#include <chrono>
#include <utility>
#include <vector>

namespace {

constexpr std::uint8_t ProtocolVersion = 1;
constexpr std::uint8_t RoomCapacity = 4;
constexpr std::size_t SnapshotEntitySize = 10;
constexpr std::size_t SnapshotMetadataSize = 25;

} // namespace

client::NetworkClient::NetworkClient()
    : _tcpHandlers{
        {server::Type::ConnectOk, &NetworkClient::onConnectOk},
        {server::Type::RoomList, &NetworkClient::onRoomList},
        {server::Type::RoomJoined, &NetworkClient::onRoomJoined},
        {server::Type::RoomUpdate, &NetworkClient::onRoomUpdate},
        {server::Type::GameStarted, &NetworkClient::onGameStarted},
        {server::Type::Error, &NetworkClient::onError},
    }
    , _udpHandlers{
        {server::UdpType::HelloAck, &NetworkClient::onHelloAck},
        {server::UdpType::Snapshot, &NetworkClient::onSnapshot},
        {server::UdpType::Event, &NetworkClient::onGameEvent},
    }
    , _work(asio::make_work_guard(this->_io))
    , _tcp(this->_io)
    , _udp(this->_io)
    , _helloTimer(this->_io)
    , _inputTimer(this->_io)
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
            server::Buffer payload{ProtocolVersion};
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

void client::NetworkClient::createRoom()
{
    server::Buffer payload;
    server::TcpProtocol::appendString(payload, this->view().username + "'s room");
    payload.push_back(RoomCapacity);
    this->sendTcp(server::TcpProtocol::message(server::Type::CreateRoom, std::move(payload)));
}

void client::NetworkClient::joinRoom(std::uint32_t id)
{
    server::Buffer payload;
    server::TcpProtocol::appendU32(payload, id);
    this->sendTcp(server::TcpProtocol::message(server::Type::JoinRoom, std::move(payload)));
}

void client::NetworkClient::leaveRoom()
{
    asio::post(this->_io, [this] {
        {
            std::scoped_lock lock(this->_mutex);
            this->_view.inRoom = false;
            this->_view.running = false;
            this->_view.host = false;
            this->_view.udpReady = false;
            this->_view.players.clear();
            this->_view.entities.clear();
            this->_view.status = "Left room";
        }
        asio::error_code ignored;
        this->_helloTimer.cancel(ignored);
        this->_inputTimer.cancel(ignored);
        this->_udp.close(ignored);
    });
    this->sendTcp(server::TcpProtocol::message(server::Type::LeaveRoom));
}

void client::NetworkClient::startGame()
{
    this->sendTcp(server::TcpProtocol::message(server::Type::StartGame));
}

void client::NetworkClient::setReady(bool ready)
{
    this->sendTcp(server::TcpProtocol::message(server::Type::SetReady, {static_cast<server::Byte>(ready)}));
}

void client::NetworkClient::setKeys(std::uint8_t keys)
{
    this->_keys = keys;
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

void client::NetworkClient::onConnectOk(const server::Buffer&, std::size_t)
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
            !server::TcpProtocol::readU8(message, offset, room.players) ||
            !server::TcpProtocol::readU8(message, offset, room.capacity) ||
            !server::TcpProtocol::readU8(message, offset, room.state))
            return;
        rooms.push_back(std::move(room));
    }
    std::scoped_lock lock(this->_mutex);
    this->_view.status = std::to_string(rooms.size()) + (rooms.size() == 1 ? " room" : " rooms") + " on the server";
    this->_view.rooms = std::move(rooms);
}

void client::NetworkClient::onRoomJoined(const server::Buffer& message, std::size_t offset)
{
    std::uint32_t id = 0;
    std::uint16_t port = 0;
    std::uint64_t token = 0;
    std::uint8_t slot = 0;
    if (!server::TcpProtocol::readU32(message, offset, id) ||
        !server::TcpProtocol::readU64(message, offset, token) ||
        !server::TcpProtocol::readU8(message, offset, slot) ||
        !server::TcpProtocol::readU16(message, offset, port))
        return;
    {
        std::scoped_lock lock(this->_mutex);
        this->_view.inRoom = true;
        this->_view.selectedRoom = id;
        this->_view.token = token;
        this->_view.slot = slot;
        this->_view.udpPort = port;
        this->_view.status = "Joined room " + std::to_string(id);
        this->_view.players.clear();
        this->_view.entities.clear();
        this->_view.collisionEffects.clear();
        this->_view.tick = 0;
        this->_view.remainingSeconds = 180;
        this->_view.gameOver = false;
        this->_view.hasGameState = false;
        this->_view.aliveMask = 0;
        this->_view.scores.fill(0);
    }
    this->startUdp(port, token);
}

void client::NetworkClient::onRoomUpdate(const server::Buffer& message, std::size_t offset)
{
    std::uint8_t host = 0;
    std::uint8_t count = 0;
    if (!server::TcpProtocol::readU8(message, offset, host) || !server::TcpProtocol::readU8(message, offset, count))
        return;
    std::vector<PlayerView> players;
    for (std::uint8_t i = 0; i < count; ++i) {
        std::uint8_t slot = 0;
        std::uint8_t bound = 0;
        std::uint8_t ready = 0;
        std::string name;
        if (!server::TcpProtocol::readU8(message, offset, slot) ||
            !server::TcpProtocol::readString(message, offset, 16, name) ||
            !server::TcpProtocol::readU8(message, offset, bound) ||
            !server::TcpProtocol::readU8(message, offset, ready))
            return;
        players.push_back({slot, std::move(name), bound != 0, ready != 0});
    }
    std::scoped_lock lock(this->_mutex);
    this->_view.host = host == this->_view.slot;
    this->_view.players = std::move(players);
}

void client::NetworkClient::onGameStarted(const server::Buffer&, std::size_t)
{
    {
        std::scoped_lock lock(this->_mutex);
        this->_view.running = true;
        this->_view.status = "Game started";
    }
    this->beginInputLoop();
}

void client::NetworkClient::onError(const server::Buffer& message, std::size_t offset)
{
    std::uint8_t code = 0;
    if (server::TcpProtocol::readU8(message, offset, code))
        this->setStatus("Server error " + std::to_string(code));
}

void client::NetworkClient::startUdp(std::uint16_t port, std::uint64_t token)
{
    asio::error_code error;
    this->_udp.close(error);
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
    std::scoped_lock lock(this->_mutex);
    if (packet.size() >= server::UdpHeaderSize + 3 && packet[server::UdpHeaderSize + 2] != 0)
        this->_view.snapshotRate = packet[server::UdpHeaderSize + 2];
    this->_view.udpReady = true;
    this->_view.status = "Ready, waiting for the host to start";
}

void client::NetworkClient::onSnapshot(const server::Buffer& packet)
{
    std::size_t offset = server::UdpHeaderSize;
    std::uint32_t tick = 0;
    std::uint16_t remainingSeconds = 0;
    std::uint8_t gameOver = 0;
    std::uint8_t aliveMask = 0;
    std::uint8_t count = 0;
    std::array<std::uint32_t, 4> scores{};
    if (!server::TcpProtocol::readU32(packet, offset, tick) ||
        !server::TcpProtocol::readU16(packet, offset, remainingSeconds) ||
        !server::TcpProtocol::readU8(packet, offset, gameOver) ||
        !server::TcpProtocol::readU8(packet, offset, aliveMask))
        return;
    for (std::uint32_t& score : scores)
        if (!server::TcpProtocol::readU32(packet, offset, score))
            return;
    if (!server::TcpProtocol::readU8(packet, offset, count) || tick <= this->_view.tick ||
        packet.size() != server::UdpHeaderSize + SnapshotMetadataSize + count * SnapshotEntitySize)
        return;
    std::vector<EntityView> entities(count);
    for (EntityView& entity : entities) {
        std::uint16_t x = 0;
        std::uint16_t y = 0;
        server::TcpProtocol::readU32(packet, offset, entity.id);
        server::TcpProtocol::readU8(packet, offset, entity.type);
        server::TcpProtocol::readU16(packet, offset, x);
        server::TcpProtocol::readU16(packet, offset, y);
        server::TcpProtocol::readU8(packet, offset, entity.flags);
        entity.x = static_cast<std::int16_t>(x);
        entity.y = static_cast<std::int16_t>(y);
    }
    std::scoped_lock lock(this->_mutex);
    this->_view.tick = tick;
    this->_view.entities = std::move(entities);
    this->_view.remainingSeconds = remainingSeconds;
    this->_view.gameOver = gameOver != 0;
    this->_view.hasGameState = true;
    this->_view.aliveMask = aliveMask;
    this->_view.scores = scores;
}

void client::NetworkClient::onGameEvent(const server::Buffer& packet)
{
    std::size_t offset = server::UdpHeaderSize;
    CollisionEffectView effect;
    std::uint32_t packedPosition = 0;
    if (!server::TcpProtocol::readU32(packet, offset, effect.eventId) ||
        !server::TcpProtocol::readU8(packet, offset, effect.type) ||
        !server::TcpProtocol::readU32(packet, offset, effect.entityId) ||
        !server::TcpProtocol::readU32(packet, offset, packedPosition) || offset != packet.size() ||
        (effect.type != static_cast<std::uint8_t>(server::CollisionEffect::RedEnemy) &&
            effect.type != static_cast<std::uint8_t>(server::CollisionEffect::BluePlayer) &&
            effect.type != static_cast<std::uint8_t>(server::CollisionEffect::RedPlayer) &&
            effect.type != static_cast<std::uint8_t>(server::CollisionEffect::EnemyDestroyed) &&
            effect.type != static_cast<std::uint8_t>(server::CollisionEffect::PlayerDestroyed)))
        return;
    effect.x = static_cast<std::int16_t>(packedPosition & 0xffffU);
    effect.y = static_cast<std::int16_t>(packedPosition >> 16);
    std::scoped_lock lock(this->_mutex);
    this->_view.collisionEffects.push_back(effect);
    if (this->_view.collisionEffects.size() > 32)
        this->_view.collisionEffects.erase(this->_view.collisionEffects.begin());
}

void client::NetworkClient::beginInputLoop()
{
    this->_inputTimer.expires_after(std::chrono::milliseconds(33));
    this->_inputTimer.async_wait([this](const asio::error_code& error) {
        if (error || !this->view().running)
            return;
        this->sendInput();
        this->beginInputLoop();
    });
}

void client::NetworkClient::sendInput()
{
    const server::Buffer payload{this->_keys.load()};
    auto bytes = std::make_shared<server::Buffer>(server::UdpProtocol::encode(server::UdpType::Input, ++this->_udpSequence, this->_view.token, payload));
    this->_udp.async_send_to(asio::buffer(*bytes), this->_udpEndpoint, [bytes](const asio::error_code&, std::size_t) {});
}

void client::NetworkClient::onDisconnect(const std::string& reason)
{
    asio::error_code ignored;
    this->_tcp.close(ignored);
    this->_udp.close(ignored);
    this->_helloTimer.cancel(ignored);
    this->_inputTimer.cancel(ignored);
    this->_writes.clear();
    std::scoped_lock lock(this->_mutex);
    this->_view.connected = false;
    this->_view.inRoom = false;
    this->_view.running = false;
    this->_view.host = false;
    this->_view.udpReady = false;
    this->_view.rooms.clear();
    this->_view.players.clear();
    this->_view.entities.clear();
    this->_view.status = "Disconnected: " + reason;
}
