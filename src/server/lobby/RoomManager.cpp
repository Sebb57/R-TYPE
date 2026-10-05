/**
 * @file RoomManager.cpp
 * @brief room manager implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "server/lobby/RoomManager.hpp"
#include "server/net/TcpConnection.hpp"
#include <algorithm>
#include <ranges>
#include <cctype>
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
    if (id == 0) 
        id = this->_nextId++;

    auto& session = this->_sessions.create(id, 0, connection->username(), connection);
    const SessionToken token = session.token;
    auto room = std::make_unique<Room>(id, name, maxPlayers, token, this->_sink, this->_conf);
    room->setPlayerName(0, connection->username());
    this->_rooms.emplace(id, std::move(room));
    connection->setSession(token);
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

    const PlayerSlot candidate = room.getInfo().players;
    auto& session = this->_sessions.create(id, candidate, connection->username(), connection);
    auto slot = room.addPlayer(session.token);
    if (!slot) {
        this->_sessions.erase(session.token);
        return std::unexpected(LobbyError::RoomFull);
    }

    session.slot = *slot;
    room.setPlayerName(*slot, connection->username());
    connection->setSession(session.token);
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
    if (room.getInfo().players < 2 || !this->_sessions.allBound(session->room))
        return std::unexpected(LobbyError::PlayersNotReady);
    room.start();
    return {};
}

void server::RoomManager::leave(SessionToken token, LeaveReason reason)
{
    Session* session = this->_sessions.find(token);
    if (session == nullptr)
        return;

    const RoomId id = session->room;
    const PlayerSlot slot = session->slot;

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
}

void server::RoomManager::push(RoomId id, InboundEvent event)
{
    auto it = this->_rooms.find(id);
    if (it != this->_rooms.end())
        it->second->push(std::move(event));
}

void server::RoomManager::sweep(Clock::time_point now)
{
    for (SessionToken token : this->_sessions.timedOut(now, this->_conf.clientTimeout))
        this->leave(token, LeaveReason::Timeout);

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
