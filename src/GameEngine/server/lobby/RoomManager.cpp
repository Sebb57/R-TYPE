/**
 * @file RoomManager.cpp
 * @brief room manager implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "GameEngine/server/lobby/RoomManager.hpp"
#include "GameEngine/server/net/TcpConnection.hpp"
#ifdef ORBYTE_FEATURE_CHAT
    #include "GameEngine/protocol/TcpProtocol.hpp"
#endif
#include <algorithm>
#include <ranges>
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

server::Result<server::RoomId> server::RoomManager::host(std::string username, std::shared_ptr<GameInstance> game)
{
    if (!game)
        return std::unexpected(LobbyError::BadRequest);
    if (username.empty() || username.size() > 16)
        return std::unexpected(LobbyError::BadName);
    if (this->_rooms.size() >= this->_conf.maxRooms)
        return std::unexpected(LobbyError::TooManyRooms);

    RoomId id = this->_nextId++;
    while (id == 0 || this->_rooms.contains(id))
        id = this->_nextId++;

    this->_rooms.emplace(id, std::make_unique<Room>(id, username + "'s game", std::move(game), this->_conf.maxSpectatorsPerRoom, this->_sink));
    std::cerr << "[ROOM] hosting id=" << id << " player=" << username << '\n';
    return id;
}

server::Result<server::JoinResult> server::RoomManager::spectate(const std::shared_ptr<TcpConnection>& connection, RoomId id)
{
    if (!connection || connection->username().empty() || connection->session())
        return std::unexpected(LobbyError::BadRequest);

    auto it = this->_rooms.find(id);
    if (it == this->_rooms.end() || it->second->isFinished())
        return std::unexpected(LobbyError::RoomNotFound);

    Room& room = *it->second;
    if (room.isFull())
        return std::unexpected(LobbyError::RoomFull);

    auto& session = this->_sessions.create(id, connection->username(), connection);
    if (!room.addSpectator(session.token)) {
        this->_sessions.erase(session.token);
        return std::unexpected(LobbyError::RoomFull);
    }

    connection->setSession(session.token);
    std::cerr << "[ROOM] spectating id=" << id << " user=" << connection->username()
        << " spectators=" << static_cast<unsigned>(room.getInfo().spectators) << '\n';
    return JoinResult(id, session.token);
}

void server::RoomManager::leave(SessionToken token, LeaveReason reason)
{
    const Session* session = this->_sessions.find(token);
    if (session == nullptr)
        return;

    std::cerr << "[ROOM] leaving id=" << session->room << " user=" << session->username
        << " reason=" << static_cast<unsigned>(reason) << '\n';

    auto it = this->_rooms.find(session->room);
    if (it != this->_rooms.end())
        it->second->removeSpectator(token);
    const auto connection = session->tcp.lock();
    this->_sessions.erase(token);
    if (connection)
        connection->left(reason);
}

void server::RoomManager::close(RoomId id)
{
    auto it = this->_rooms.find(id);
    if (it == this->_rooms.end())
        return;

    const std::vector<SessionToken> spectators = it->second->getSpectators();
    for (SessionToken token : spectators)
        this->leave(token, LeaveReason::GameEnded);
    this->_rooms.erase(it);
    std::cerr << "[ROOM] closed id=" << id << '\n';
}

void server::RoomManager::sweep(Clock::time_point now)
{
    for (SessionToken token : this->_sessions.timedOut(now, this->_conf.clientTimeout))
        this->leave(token, LeaveReason::Timeout);

    std::vector<RoomId> finished;
    for (const auto& [id, room] : this->_rooms)
        if (room->isFinished())
            finished.push_back(id);
    for (RoomId id : finished)
        this->close(id);
}

void server::RoomManager::stopAll()
{
    std::vector<RoomId> ids;
    ids.reserve(this->_rooms.size());
    for (const auto& [id, room] : this->_rooms)
        ids.push_back(id);
    for (RoomId id : ids)
        this->close(id);
}

#ifdef ORBYTE_FEATURE_CHAT
void server::RoomManager::chat(SessionToken token, std::string text)
{
    const Session* session = this->_sessions.find(token);
    if (session != nullptr)
        this->chat(session->room, ChatLine{session->username, std::move(text), false});
}

void server::RoomManager::chat(RoomId id, ChatLine line)
{
    const auto it = this->_rooms.find(id);
    if (it == this->_rooms.end())
        return;

    Buffer message{static_cast<Byte>(Type::Chat), static_cast<Byte>(line.host ? 1 : 0)};
    TcpProtocol::appendString(message, line.author);
    TcpProtocol::appendString(message, line.text);
    for (SessionToken token : it->second->getSpectators()) {
        const Session* session = this->_sessions.find(token);
        if (session == nullptr)
            continue;
        if (const auto connection = session->tcp.lock())
            connection->send(message);
    }
    if (this->_chatSink)
        this->_chatSink(id, line);
}
#endif
