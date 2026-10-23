/**
 * @file SessionRegistry.hpp
 * @brief session registry class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_LOBBY_SESSIONREGISTRY_HPP
#define GAMEENGINE_SERVER_LOBBY_SESSIONREGISTRY_HPP

#include <asio.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "GameEngine/server/common/ServerConfig.hpp"

namespace server {

class TcpConnection;

struct Session {
    SessionToken token;
    RoomId room;
    std::string username;
    std::weak_ptr<TcpConnection> tcp;
    std::optional<asio::ip::udp::endpoint> udp;
    Clock::time_point lastSeen;
};

class SessionRegistry {
    public:
        Session& create(RoomId, std::string username, std::weak_ptr<TcpConnection>);
        Session* find(SessionToken);
        bool bindEndpoint(SessionToken, const asio::ip::udp::endpoint&);
        void touch(SessionToken, Clock::time_point now);
        void erase(SessionToken);
        [[nodiscard]] std::vector<SessionToken> timedOut(Clock::time_point now, std::chrono::seconds timeout) const;
        [[nodiscard]] std::vector<asio::ip::udp::endpoint> endpointsOf(RoomId) const;

    private:
        std::unordered_map<SessionToken, Session> _sessions;
};

} // namespace server

#endif // GAMEENGINE_SERVER_LOBBY_SESSIONREGISTRY_HPP
