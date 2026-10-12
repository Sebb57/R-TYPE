/**
 * @file UdpServer.cpp
 * @brief udp server implementation
 *
 * @author Seb
 * @date 2026-10-12
**/

#include "server/net/UdpServer.hpp"
#include "server/lobby/RoomManager.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>

server::UdpServer::UdpServer(asio::io_context& io, SessionRegistry& sessions, RoomManager& rooms, const ServerConfig& config)
    : _socket(io), _sessions(sessions), _rooms(rooms), _port(config.udpPort),
      _tickRate(static_cast<std::uint8_t>(std::min(config.tickRate, 255U))),
      _snapshotRate(static_cast<std::uint8_t>(std::min(config.snapshotRate, 255U)))
{}

void server::UdpServer::start()
{
    const asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), this->_port);
    this->_socket.open(endpoint.protocol());
    this->_socket.bind(endpoint);
    this->doReceive();
}

void server::UdpServer::stop()
{
    asio::error_code ignored;
    this->_socket.cancel(ignored);
    this->_socket.close(ignored);
}

void server::UdpServer::doReceive()
{
    if (!this->_socket.is_open())
        return;
    this->_socket.async_receive_from(asio::buffer(this->_rx), this->_sender,
        [this](const asio::error_code& error, std::size_t size) {
            if (!error)
                this->onDatagram(this->_sender, size);
            if (this->_socket.is_open())
                this->doReceive();
        });
}

void server::UdpServer::onDatagram(const asio::ip::udp::endpoint& from, std::size_t size)
{
    auto header = UdpProtocol::parseHeader(this->_rx, size);
    if (!header || header->type == UdpType::Snapshot || header->type == UdpType::Event || header->type == UdpType::Pong)
        return;
    Session* session = this->_sessions.find(header->token);
    if (session == nullptr)
        return;

    if (header->type == UdpType::Hello) {
        if (header->payloadSize == 0)
            this->onHello(*session, from);
        return;
    }
    if (!session->udp || *session->udp != from)
        return;
    switch (header->type) {
        case UdpType::Input: this->onInput(*session, *header, size); break;
        case UdpType::Ping: this->onPing(*session, *header, size); break;
        case UdpType::Disconnect:
            if (header->payloadSize == 1) {
                this->_sessions.touch(header->token, Clock::now());
                this->_rooms.leave(header->token, LeaveReason::Requested);
            }
            break;
        default: break;
    }
}

void server::UdpServer::onHello(Session& session, const asio::ip::udp::endpoint& from)
{
    if (!this->_sessions.bindEndpoint(session.token, from))
        return;
    this->_rooms.refreshRoom(session.room);
    Buffer payload{session.slot, this->_tickRate, this->_snapshotRate};
    auto bytes = std::make_shared<const Buffer>(UdpProtocol::encode(UdpType::HelloAck, 0, 0, payload));
    this->sendRaw(from, std::move(bytes));
}

void server::UdpServer::onInput(Session& session, const UdpHeader& header, std::size_t size)
{
    if (size != UdpHeaderSize + 1 || (session.lastInputSeq &&
        static_cast<std::int32_t>(header.sequence - *session.lastInputSeq) <= 0))
        return;
    session.lastInputSeq = header.sequence;
    this->_sessions.touch(session.token, Clock::now());
    this->_rooms.push(session.room, PlayerInput{session.slot, static_cast<std::uint8_t>(this->_rx[UdpHeaderSize] & 0x1f)});
}

void server::UdpServer::onPing(Session& session, const UdpHeader& header, std::size_t size)
{
    if (size != UdpHeaderSize + 4)
        return;
    this->_sessions.touch(session.token, Clock::now());
    Buffer payload(this->_rx.begin() + static_cast<std::ptrdiff_t>(UdpHeaderSize), this->_rx.begin() + static_cast<std::ptrdiff_t>(size));
    auto bytes = std::make_shared<const Buffer>(UdpProtocol::encode(UdpType::Pong, header.sequence, 0, payload));
    this->sendRaw(*session.udp, std::move(bytes));
}

void server::UdpServer::send(const OutboundMessage& message)
{
    UdpType type = std::holds_alternative<SnapshotDto>(message.payload) ? UdpType::Snapshot : UdpType::Event;
    std::uint32_t sequence = 0;
    Buffer payload;
    if (const auto* snapshot = std::get_if<SnapshotDto>(&message.payload)) {
        sequence = snapshot->tick;
        UdpProtocol::appendU32(payload, snapshot->tick);
        const auto count = std::min<std::size_t>(snapshot->entities.size(), 100);
        payload.push_back(static_cast<Byte>(count));
        for (std::size_t i = 0; i < count; ++i) {
            const auto& entity = snapshot->entities[i];
            UdpProtocol::appendU32(payload, entity.id);
            payload.push_back(entity.type);
            payload.push_back(static_cast<Byte>(entity.x)); payload.push_back(static_cast<Byte>(static_cast<std::uint16_t>(entity.x) >> 8));
            payload.push_back(static_cast<Byte>(entity.y)); payload.push_back(static_cast<Byte>(static_cast<std::uint16_t>(entity.y) >> 8));
            payload.push_back(entity.flags);
        }
    } else {
        const auto& event = std::get<GameEventDto>(message.payload);
        sequence = event.eventId;
        UdpProtocol::appendU32(payload, event.eventId);
        payload.push_back(event.eventType);
        UdpProtocol::appendU32(payload, event.entityId);
        UdpProtocol::appendU32(payload, event.extra);
    }
    auto bytes = std::make_shared<const Buffer>(UdpProtocol::encode(type, sequence, 0, payload));
    for (const auto& endpoint : this->_sessions.endpointsOf(message.room, message.target))
        this->sendRaw(endpoint, bytes);
}

void server::UdpServer::sendRaw(const asio::ip::udp::endpoint& endpoint, std::shared_ptr<const Buffer> bytes)
{
    if (!this->_socket.is_open() || bytes->empty())
        return;
    this->_socket.async_send_to(asio::buffer(*bytes), endpoint,
        [bytes](const asio::error_code&, std::size_t) {});
}
