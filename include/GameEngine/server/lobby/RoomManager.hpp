/**
 * @file RoomManager.hpp
 * @brief room manager class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_LOBBY_ROOMMANAGER_HPP
#define GAMEENGINE_SERVER_LOBBY_ROOMMANAGER_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "GameEngine/server/common/ServerConfig.hpp"
#include "GameEngine/server/common/ServerMessages.hpp"
#include "GameEngine/server/lobby/SessionRegistry.hpp"
#include "GameEngine/server/lobby/Room.hpp"

namespace server {

class TcpConnection;
struct JoinResult { RoomId room; SessionToken token; };

class RoomManager {
    public:
        RoomManager(SessionRegistry&, OutgoingSink, const ServerConfig&);

        [[nodiscard]] std::vector<RoomInfo> list() const;
        Result<RoomId> host(std::string username, std::shared_ptr<GameInstance> game);
        Result<JoinResult> spectate(const std::shared_ptr<TcpConnection>&, RoomId);

        void leave(SessionToken, LeaveReason);
        void close(RoomId);
        void sweep(Clock::time_point now);
        void stopAll();

    private:
        std::unordered_map<RoomId, std::unique_ptr<Room>> _rooms;
        RoomId _nextId = 1;
        SessionRegistry& _sessions;
        OutgoingSink _sink;
        const ServerConfig& _conf;
};

} // namespace server

#endif // GAMEENGINE_SERVER_LOBBY_ROOMMANAGER_HPP
