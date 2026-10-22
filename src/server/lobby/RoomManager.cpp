/**
 * @file RoomManager.cpp
 * @brief room manager implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "server/lobby/RoomManager.hpp"
#include "server/net/TcpConnection.hpp"
#include "protocol/TcpProtocol.hpp"
#include <algorithm>
#include <ranges>
#include <cctype>
#include <iostream>
#include <utility>

server::RoomManager::RoomManager(SessionRegistry& sessions, OutgoingSink sink, const ServerConfig& conf)
    : _sessions(sessions)
    , _sink(std::move(sink))
    , _conf(conf)
{}

std::vector<server::RoomInfo> server::RoomManager::list() const
{
    std::vector<RoomInfo> result;

    result.reserve(this->_rooms.size());
    for (const auto& [id, room] : this->_rooms)
        result.push_back(room->getInfo());

    std::ranges::sort(result.begin(), result.end(), [](const RoomInfo& a, const RoomInfo& b) { return a.id < b.id; });
    return result;
}

server::Result<server::JoinResult> server::RoomManager::create(const std::shared_ptr<TcpConnection>& connection, std::string name, std::uint8_t maxPlayers)
{
    if (!connection || connection->username().empty() || connection->session())
        return std::unexpected(LobbyError::NotInRoom);

    const bool validName = !name.empty() && name.size() <= 32 && std::ranges::any_of(name.begin(), name.end(), [](unsigned char c) { return !std::isspace(c); });
    if (!validName || maxPlayers == 0 || maxPlayers > this->_conf.maxPlayersPerRoom || maxPlayers > 4)
        return std::unexpected(LobbyError::BadName);
    if (this->_rooms.size() >= this->_conf.maxRooms)
        return std::unexpected(LobbyError::TooManyRooms);

    RoomId id = this->_nextId++;
    while (id == 0 || this->_rooms.contains(id))
        id = this->_nextId++;

    auto& session = this->_sessions.create(id, 0, connection->username(), connection);
    const SessionToken token = session.token;
    auto room = std::make_unique<Room>(id, name, maxPlayers, token, this->_sink, this->_conf);
    room->setPlayerName(0, connection->username());
    this->_rooms.emplace(id, std::move(room));
    connection->setSession(token);
    std::cerr << "[ROOM] created id=" << id << " name=" << name << " host=" << connection->username() << '\n';
    return JoinResult(id, token, 0);
}

server::Result<server::JoinResult> server::RoomManager::join(const std::shared_ptr<TcpConnection>& connection, RoomId id)
{
    if (!connection || connection->username().empty() || connection->session())
        return std::unexpected(LobbyError::NotInRoom);

    auto it = this->_rooms.find(id);
    if (it == this->_rooms.end()) 
        return std::unexpected(LobbyError::RoomNotFound);

    Room& room = *it->second;
    if (room.getState() != RoomState::Lobby)
        return std::unexpected(LobbyError::AlreadyStarted);
    if (room.isFull())
        return std::unexpected(LobbyError::RoomFull);

    auto& session = this->_sessions.create(id, 0, connection->username(), connection);
    auto slot = room.addPlayer(session.token);
    if (!slot) {
        this->_sessions.erase(session.token);
        return std::unexpected(LobbyError::RoomFull);
    }

    session.slot = *slot;
    room.setPlayerName(*slot, connection->username());
    connection->setSession(session.token);
    if (room.getState() == RoomState::Running)
        room.push(PlayerJoined(*slot, connection->username()));
    std::cerr << "[ROOM] joined id=" << id << " slot=" << static_cast<unsigned>(*slot)
        << " user=" << connection->username() << " players=" << static_cast<unsigned>(room.getInfo().players) << '\n';
    return JoinResult(id, session.token, *slot);
}

server::Result<void> server::RoomManager::startGame(SessionToken requester)
{
    Session* session = this->_sessions.find(requester);
    if (session == nullptr)
        return std::unexpected(LobbyError::NotInRoom);

    auto it = this->_rooms.find(session->room);
    if (it == this->_rooms.end())
        return std::unexpected(LobbyError::RoomNotFound);

    Room& room = *it->second;
    if (room.getState() != RoomState::Lobby)
        return std::unexpected(LobbyError::AlreadyStarted);
    if (room.getHost() != requester)
        return std::unexpected(LobbyError::NotHost);
    if (!room.allReady() || !this->_sessions.allBound(session->room))
        return std::unexpected(LobbyError::PlayersNotReady);
    room.start();
    std::cerr << "[ROOM] started id=" << session->room << " players=" << static_cast<unsigned>(room.getInfo().players) << '\n';
    const Buffer started{static_cast<Byte>(Type::GameStarted)};
    for (const Session* player : this->_sessions.sessionsIn(session->room))
        if (auto connection = player->tcp.lock())
            connection->send(started);
    return {};
}

server::Result<void> server::RoomManager::setReady(SessionToken requester, bool ready)
{
    Session* session = this->_sessions.find(requester);
    if (session == nullptr)
        return std::unexpected(LobbyError::NotInRoom);
    auto room = this->_rooms.find(session->room);
    if (room == this->_rooms.end())
        return std::unexpected(LobbyError::RoomNotFound);
    if (room->second->getState() != RoomState::Lobby)
        return std::unexpected(LobbyError::AlreadyStarted);
    room->second->setReady(session->slot, ready);
    this->notifyRoom(session->room);
    return {};
}

bool server::RoomManager::isRunning(RoomId id) const
{
    const auto room = this->_rooms.find(id);
    return room != this->_rooms.end() && room->second->getState() == RoomState::Running;
}

void server::RoomManager::leave(SessionToken token, LeaveReason reason)
{
    Session* session = this->_sessions.find(token);
    if (session == nullptr)
        return;

    const RoomId id = session->room;
    const PlayerSlot slot = session->slot;
    const std::string username = session->username;
    std::cerr << "[ROOM] leaving id=" << id << " slot=" << static_cast<unsigned>(slot)
        << " user=" << username << " reason=" << static_cast<unsigned>(reason) << '\n';

    auto it = this->_rooms.find(id);
    if (it != this->_rooms.end()) {
        it->second->push(PlayerLeft(slot, reason));
        it->second->removePlayer(slot);
        if (it->second->isEmpty()) {
            it->second->stop();
            this->_rooms.erase(it);
        }
    }
    this->_sessions.erase(token);
    if (this->_rooms.contains(id))
        this->notifyRoom(id);
}

void server::RoomManager::notifyRoom(RoomId id)
{
    const auto roomIt = this->_rooms.find(id);
    if (roomIt == this->_rooms.end())
        return;
    const RoomInfo info = roomIt->second->getInfo();
    const auto host = roomIt->second->slotOf(roomIt->second->getHost()).value_or(0);
    Buffer message{static_cast<Byte>(Type::RoomUpdate), host, info.players};
    const auto sessions = this->_sessions.sessionsIn(id);
    for (const Session* session : sessions) {
        message.push_back(session->slot);
        TcpProtocol::appendString(message, session->username);
        message.push_back(session->udp.has_value() ? 1 : 0);
        message.push_back(roomIt->second->isReady(session->slot) ? 1 : 0);
    }
    for (const Session* session : sessions) {
        if (auto connection = session->tcp.lock())
            connection->send(message);
    }
}

void server::RoomManager::push(RoomId id, InboundEvent event)
{
    auto it = this->_rooms.find(id);
    if (it != this->_rooms.end())
        it->second->push(std::move(event));
}

void server::RoomManager::refreshRoom(RoomId id)
{
    notifyRoom(id);
}

void server::RoomManager::sweep(Clock::time_point now)
{
    for (SessionToken token : this->_sessions.timedOut(now, this->_conf.clientTimeout)) {
        const Session* session = this->_sessions.find(token);
        if (session == nullptr)
            continue;
        const auto room = this->_rooms.find(session->room);
        if (room != this->_rooms.end() && room->second->getState() == RoomState::Running)
            this->leave(token, LeaveReason::Timeout);
    }

    for (auto it = this->_rooms.begin(); it != this->_rooms.end();) {
        it->second->refresh();
        if (it->second->getState() == RoomState::Running && it->second->isEmpty()) {
            it->second->stop();
            it = this->_rooms.erase(it);
        } else {
            it++;
        }
    }
}

void server::RoomManager::stopAll()
{
    for (auto& [id, room] : this->_rooms)
        room->stop();
    this->_rooms.clear();
}
