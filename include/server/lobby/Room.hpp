/**
 * @file Room.hpp
 * @brief room class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_LOBBY_ROOM_HPP
#define SERVER_LOBBY_ROOM_HPP

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
        Room(RoomId, std::string name, std::uint8_t maxPlayers, SessionToken host, OutgoingSink, const ServerConfig& conf);

        std::optional<PlayerSlot> addPlayer(SessionToken token);
        void setPlayerName(PlayerSlot slot, std::string name);
        void removePlayer(PlayerSlot);
        [[nodiscard]] bool isFull() const;
        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] bool contains(SessionToken) const;
        [[nodiscard]] std::optional<PlayerSlot> slotOf(SessionToken) const;
        [[nodiscard]] SessionToken getHost() const { return this->_host; }
        void promoteNextHost();
        void start();
        void stop();
        void push(InboundEvent event);
        [[nodiscard]] RoomState getState() const { return this->_state; }
        [[nodiscard]] RoomInfo getInfo() const;
        void refresh();

    private:
        RoomId _id;
        RoomState _state = RoomState::Lobby;
        std::string _name;
        std::array<std::optional<SessionToken>, 4> _slots;
        std::array<std::string, 4> _playerNames;
        SessionToken _host;
        std::unique_ptr<GameInstance> _game;
        std::uint8_t _maxPlayers;
        const ServerConfig& _conf;
        OutgoingSink _sink;
};

} // namespace server

#endif // SERVER_LOBBY_ROOM_HPP
