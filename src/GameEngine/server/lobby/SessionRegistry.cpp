/**
 * @file SessionRegistry.cpp
 * @brief session registry implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include <random>
#include "GameEngine/server/lobby/SessionRegistry.hpp"
#include "GameEngine/server/common/ServerConfig.hpp"

namespace {

server::SessionToken generateRandomToken()
{
    static std::random_device rd;
    const auto high = static_cast<server::SessionToken>(rd());
    const auto low  = static_cast<server::SessionToken>(rd());

    return (high << 32) | low;
}

} // namespace

server::Session& server::SessionRegistry::create(RoomId room, std::string username, std::weak_ptr<TcpConnection> tcp)
{
    SessionToken token = generateRandomToken();
    while (token == 0 || this->_sessions.contains(token))
        token = generateRandomToken();

    auto [it, inserted] = this->_sessions.emplace(token, Session(token, room,
        std::move(username), std::move(tcp), std::nullopt, Clock::now()));
    return it->second;
}

server::Session* server::SessionRegistry::find(SessionToken token)
{
    auto it = this->_sessions.find(token);
    return it == this->_sessions.end() ? nullptr : &it->second;
}

bool server::SessionRegistry::bindEndpoint(SessionToken token, const asio::ip::udp::endpoint& endpoint)
{
    Session* session = this->find(token);
    if (session == nullptr)
        return false;

    if (session->udp && *session->udp != endpoint)
        return false;
    session->udp = endpoint;
    session->lastSeen = Clock::now();
    return true;
}

void server::SessionRegistry::touch(SessionToken token, Clock::time_point now)
{
    if (Session* session = this->find(token))
        session->lastSeen = now;
}

void server::SessionRegistry::erase(SessionToken token)
{
    this->_sessions.erase(token);
}

std::vector<server::SessionToken> server::SessionRegistry::timedOut(Clock::time_point now, std::chrono::seconds timeout) const
{
    std::vector<SessionToken> result;

    for (const auto& [token, session] : this->_sessions)
        if (now - session.lastSeen >= timeout)
            result.push_back(token);
    return result;
}

std::vector<asio::ip::udp::endpoint> server::SessionRegistry::endpointsOf(RoomId room) const
{
    std::vector<asio::ip::udp::endpoint> result;

    for (const auto& [token, session] : this->_sessions)
        if (session.room == room && session.udp)
            result.push_back(*session.udp);
    return result;
}
