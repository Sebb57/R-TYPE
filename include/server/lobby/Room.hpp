/**
 * @file Room.hpp
 * @brief room class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef ROOM_HPP_
#define ROOM_HPP_

#include <array>
#include <memory>
#include <optional>
#include <string>
#include "server/common/ServerConfig.hpp"
#include "server/common/ServerMessages.hpp"
#include "server/game/GameInstance.hpp"

namespace server {

class Room {
    public:
        Room(RoomId, std::string name, std::uint8_t maxPlayers, SessionToken host, OutgoingSink, const ServerConfig&);

        std::optional<PlayerSlot> addPlayer(SessionToken);
        void removePlayer(PlayerSlot);
        [[nodiscard]] bool isFull() const;
        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] SessionToken getHost() const;
        void promoteNextHost();
        void start();
        void stop();
        void push(InboundEvent);
        [[nodiscard]] RoomState getState() const;
        [[nodiscard]] RoomInfo getInfo() const;

    private:
        RoomId _id;
        std::string _name;
        std::array<std::optional<SessionToken>, 4> _slots;
        SessionToken _host;
        std::unique_ptr<GameInstance> _game;
        std::uint8_t _maxPlayers;
        const ServerConfig& _conf;
};

}

#endif /* ROOM_HPP_ */
