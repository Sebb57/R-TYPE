/**
 * @file Room.hpp
 * @brief room class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_LOBBY_ROOM_HPP
#define GAMEENGINE_SERVER_LOBBY_ROOM_HPP

#include <array>
#include <memory>
#include <optional>
#include <string>
#include "GameEngine/server/common/ServerConfig.hpp"
#include "GameEngine/server/common/ServerMessages.hpp"
#include "GameEngine/server/game/GameInstance.hpp"

namespace server {

class Room {
    public:
        Room(RoomId, std::string name, std::uint8_t maxPlayers, SessionToken host, OutgoingSink, const ServerConfig& conf);

        std::optional<PlayerSlot> addPlayer(SessionToken token);
        void setPlayerName(PlayerSlot slot, std::string name);
        void setReady(PlayerSlot slot, bool ready);
        [[nodiscard]] bool isReady(PlayerSlot slot) const;
        [[nodiscard]] bool allReady() const;
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
        std::array<bool, 4> _ready{};
        SessionToken _host;
        std::unique_ptr<GameInstance> _game;
        std::uint8_t _maxPlayers;
        const ServerConfig& _conf;
        OutgoingSink _sink;
};

} // namespace server

#endif // GAMEENGINE_SERVER_LOBBY_ROOM_HPP
