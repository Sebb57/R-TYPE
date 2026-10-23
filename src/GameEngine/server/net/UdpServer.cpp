/**
 * @file UdpServer.cpp
 * @brief udp server implementation
 *
 * @author Seb
 * @date 2026-10-12
**/

#include "GameEngine/server/net/UdpServer.hpp"
#include "GameEngine/protocol/TcpProtocol.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr std::size_t MaxSnapshotPayload = server::MaxUdpDatagram - server::UdpHeaderSize;
constexpr std::size_t MaxSnapshotParts = 255;
static_assert((MaxSnapshotPayload - server::SnapshotHeaderSize) / server::SnapshotEntitySize < server::NoImage);

struct SnapshotPart {
    std::vector<std::string> images;
    std::vector<std::pair<const server::EntityDto*, std::uint8_t>> entities;
    std::size_t size = server::SnapshotHeaderSize;
};

template <class T>
T clampTo(float value)
{
    return static_cast<T>(std::lround(std::clamp(value, static_cast<float>(std::numeric_limits<T>::min()), static_cast<float>(std::numeric_limits<T>::max()))));
}

bool sendsImage(const server::EntityDto& entity)
{
    return !entity.image.empty() && entity.image.size() <= server::MaxImagePath;
}

std::size_t cost(const SnapshotPart& part, const server::EntityDto& entity)
{
    if (!sendsImage(entity) || std::ranges::find(part.images, entity.image) != part.images.end())
        return server::SnapshotEntitySize;
    return server::SnapshotEntitySize + 1 + entity.image.size();
}

std::vector<SnapshotPart> split(const server::SnapshotDto& snapshot)
{
    std::vector<SnapshotPart> parts(1);
    for (const server::EntityDto& entity : snapshot.entities) {
        if (parts.back().size + cost(parts.back(), entity) > MaxSnapshotPayload) {
            if (parts.size() == MaxSnapshotParts)
                break;
            parts.emplace_back();
        }
        SnapshotPart& part = parts.back();
        part.size += cost(part, entity);
        std::uint8_t index = server::NoImage;
        if (sendsImage(entity)) {
            const auto image = std::ranges::find(part.images, entity.image);
            index = static_cast<std::uint8_t>(image - part.images.begin());
            if (image == part.images.end())
                part.images.push_back(entity.image);
        }
        part.entities.emplace_back(&entity, index);
    }
    return parts;
}

server::Buffer encode(std::uint32_t tick, std::size_t index, std::size_t count, const SnapshotPart& part)
{
    server::Buffer payload;
    payload.reserve(part.size);
    server::UdpProtocol::appendU32(payload, tick);
    payload.push_back(static_cast<server::Byte>(index));
    payload.push_back(static_cast<server::Byte>(count));
    payload.push_back(static_cast<server::Byte>(part.images.size()));
    for (const std::string& image : part.images)
        server::TcpProtocol::appendString(payload, image);
    payload.push_back(static_cast<server::Byte>(part.entities.size()));
    for (const auto& [entity, image] : part.entities) {
        server::UdpProtocol::appendU32(payload, entity->id);
        server::TcpProtocol::appendU16(payload, static_cast<std::uint16_t>(clampTo<std::int16_t>(entity->position.x)));
        server::TcpProtocol::appendU16(payload, static_cast<std::uint16_t>(clampTo<std::int16_t>(entity->position.y)));
        server::TcpProtocol::appendU16(payload, static_cast<std::uint16_t>(clampTo<std::int16_t>(std::fmod(entity->rotation, 360.0f))));
        server::TcpProtocol::appendU16(payload, clampTo<std::uint16_t>(entity->size.x));
        server::TcpProtocol::appendU16(payload, clampTo<std::uint16_t>(entity->size.y));
        payload.insert(payload.end(), {entity->color.r, entity->color.g, entity->color.b, entity->color.a});
        payload.push_back(image);
    }
    return payload;
}

} // namespace

server::UdpServer::UdpServer(asio::io_context& io, SessionRegistry& sessions, const ServerConfig& config)
    : _socket(io), _sessions(sessions), _port(config.udpPort),
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
    if (!header || (header->type != UdpType::Hello && header->type != UdpType::Ping))
        return;
    Session* session = this->_sessions.find(header->token);
    if (session == nullptr)
        return;

    if (header->type == UdpType::Hello) {
        if (header->payloadSize == 0)
            this->onHello(*session, from);
        return;
    }
    if (session->udp && *session->udp == from)
        this->onPing(*session, *header, size);
}

void server::UdpServer::onHello(Session& session, const asio::ip::udp::endpoint& from)
{
    if (!this->_sessions.bindEndpoint(session.token, from))
        return;
    std::cerr << "[UDP] bound user=" << session.username << " room=" << session.room << " peer=" << from << '\n';
    Buffer payload{this->_tickRate, this->_snapshotRate};
    auto bytes = std::make_shared<const Buffer>(UdpProtocol::encode(UdpType::HelloAck, 0, 0, payload));
    this->sendRaw(from, std::move(bytes));
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
    const SnapshotDto& snapshot = message.snapshot;
    const auto endpoints = this->_sessions.endpointsOf(message.room);
    std::size_t& clients = this->_lastSnapshotClients[message.room];
    if (clients != endpoints.size()) {
        std::cerr << "[UDP] snapshot room=" << message.room << " tick=" << snapshot.tick
            << " entities=" << snapshot.entities.size() << " clients=" << endpoints.size() << '\n';
        clients = endpoints.size();
    }
    if (endpoints.empty())
        return;
    const auto parts = split(snapshot);
    for (std::size_t index = 0; index < parts.size(); ++index) {
        auto bytes = std::make_shared<const Buffer>(UdpProtocol::encode(UdpType::Snapshot, snapshot.tick, 0,
            encode(snapshot.tick, index, parts.size(), parts[index])));
        for (const auto& endpoint : endpoints)
            this->sendRaw(endpoint, bytes);
    }
}

void server::UdpServer::sendRaw(const asio::ip::udp::endpoint& endpoint, std::shared_ptr<const Buffer> bytes)
{
    if (!this->_socket.is_open() || bytes->empty())
        return;
    this->_socket.async_send_to(asio::buffer(*bytes), endpoint,
        [bytes](const asio::error_code&, std::size_t) {});
}
