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
struct JoinResult { RoomId room; SessionToken token; PlayerSlot slot; };

class RoomManager {
    public:
        RoomManager(SessionRegistry&, OutgoingSink, const ServerConfig&);

        std::vector<RoomInfo> list() const;
        Result<JoinResult> host(std::string username, OutgoingSink local);
        Result<JoinResult> join(const std::shared_ptr<TcpConnection>&, RoomId);
        Result<void> startGame(SessionToken requester);
        Result<void> setReady(SessionToken requester, bool ready);
        [[nodiscard]] bool isRunning(RoomId) const;

        void leave(SessionToken, LeaveReason);
        void push(RoomId, InboundEvent);
        void refreshRoom(RoomId);
        void sweep(Clock::time_point now);
        void stopAll();

    private:
        void notifyRoom(RoomId);
        std::unordered_map<RoomId, std::unique_ptr<Room>> _rooms;
        RoomId _nextId = 1;
        SessionRegistry& _sessions;
        OutgoingSink _sink;
        const ServerConfig& _conf;
};

} // namespace server

#endif // GAMEENGINE_SERVER_LOBBY_ROOMMANAGER_HPP
