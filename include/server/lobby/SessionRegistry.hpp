/**
 * @file SessionRegistry.hpp
 * @brief session registry class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SESSIONREGISTRY_HPP_
#define SESSIONREGISTRY_HPP_

#include <asio.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "server/common/ServerConfig.hpp"
#include "server/common/ServerMessages.hpp"

namespace server {

class TcpConnection;

struct Session {
    SessionToken token;
    RoomId room;
    PlayerSlot slot;
    std::string username;
    std::weak_ptr<TcpConnection> tcp;
    std::optional<asio::ip::udp::endpoint> udp;
    Clock::time_point lastSeen;
    std::optional<std::uint32_t> lastInputSeq;
};

class SessionRegistry {
    public:
        Session& create(RoomId, PlayerSlot, std::string username, std::weak_ptr<TcpConnection>);
        Session* find(SessionToken);
        bool bindEndpoint(SessionToken, const asio::ip::udp::endpoint&);
        void touch(SessionToken, Clock::time_point now);
        void erase(SessionToken);
        [[nodiscard]] std::vector<SessionToken> timedOut(Clock::time_point now, std::chrono::seconds timeout) const;
        [[nodiscard]] std::vector<asio::ip::udp::endpoint> endpointsOf(RoomId, const Target&) const;
        [[nodiscard]] bool allBound(RoomId) const;

    private:
        std::unordered_map<SessionToken, Session> _sessions;
        SessionToken _nextToken = 1;
};

}

#endif /* SESSIONREGISTRY_HPP_ */
