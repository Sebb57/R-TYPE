/**
 * @file SessionRegistry.cpp
 * @brief session registry implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include <random>
#include <ranges>
#include "server/lobby/SessionRegistry.hpp"
#include "ServerConfig.hpp"

namespace {

server::SessionToken generateRandomToken()
{
    static std::random_device rd;
    const auto high = static_cast<server::SessionToken>(rd());
    const auto low  = static_cast<server::SessionToken>(rd());

    return (high << 32) | low;
}

} // namespace

server::Session& server::SessionRegistry::create(RoomId room, PlayerSlot slot, std::string username, std::weak_ptr<TcpConnection> tcp)
{
    SessionToken token = generateRandomToken();
    while (token == 0 || this->_sessions.contains(token))
        token = generateRandomToken();

    auto [it, inserted] = this->_sessions.emplace(token, Session(token, room, slot,
        std::move(username), std::move(tcp), std::nullopt, Clock::now(), std::nullopt, std::nullopt));
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

std::vector<asio::ip::udp::endpoint> server::SessionRegistry::endpointsOf(RoomId room, const Target& target) const
{
    std::vector<asio::ip::udp::endpoint> result;

    for (const auto& [token, session] : this->_sessions) {
        if (session.room != room || !session.udp)
            continue;
        const bool include = target.kind == Target::Kind::All || (target.kind == Target::Kind::Only && session.slot == target.slot) || (target.kind == Target::Kind::AllExcept && session.slot != target.slot);
        if (include)
            result.push_back(*session.udp);
    }
    return result;
}

bool server::SessionRegistry::allBound(RoomId room) const
{
    bool found = false;

    for (const auto& [token, session] : this->_sessions) {
        if (session.room != room)
            continue;
        found = true;
        if (!session.udp)
            return false;
    }
    return found;
}

std::vector<const server::Session*> server::SessionRegistry::sessionsIn(RoomId room) const
{
    std::vector<const Session*> result;
    for (const auto& [token, session] : _sessions)
        if (session.room == room)
            result.push_back(&session);
    std::ranges::sort(result, {}, [](const Session* session) { return session->slot; });
    return result;
}
