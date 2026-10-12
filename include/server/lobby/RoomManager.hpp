/**
 * @file RoomManager.hpp
 * @brief room manager class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_LOBBY_ROOMMANAGER_HPP
#define SERVER_LOBBY_ROOMMANAGER_HPP

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "server/common/ServerConfig.hpp"
#include "server/common/ServerMessages.hpp"
#include "server/lobby/SessionRegistry.hpp"
#include "server/lobby/Room.hpp"

namespace server {

class TcpConnection;
struct JoinResult { RoomId room; SessionToken token; PlayerSlot slot; };

class RoomManager {
    public:
        RoomManager(SessionRegistry&, OutgoingSink, const ServerConfig&);

        std::vector<RoomInfo> list() const;
        Result<JoinResult> create(const std::shared_ptr<TcpConnection>&, std::string roomName, std::uint8_t maxPlayers);
        Result<JoinResult> join(const std::shared_ptr<TcpConnection>&, RoomId);
        Result<void> startGame(SessionToken requester);

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

#endif // SERVER_LOBBY_ROOMMANAGER_HPP
