/**
 * @file Room.hpp
 * @brief room class, a hosted solo game and the spectators watching it
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_LOBBY_ROOM_HPP
#define GAMEENGINE_SERVER_LOBBY_ROOM_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "GameEngine/server/common/ServerConfig.hpp"
#include "GameEngine/server/common/ServerMessages.hpp"
#include "GameEngine/server/game/GameInstance.hpp"

namespace server {

class Room {
    public:
        Room(RoomId, std::string name, std::shared_ptr<GameInstance> game, std::uint8_t maxSpectators, OutgoingSink);
        ~Room();

        Room(const Room&) = delete;
        Room& operator=(const Room&) = delete;
        Room(Room&&) = delete;
        Room& operator=(Room&&) = delete;

        bool addSpectator(SessionToken token);
        void removeSpectator(SessionToken token);
        [[nodiscard]] bool isFull() const;
        [[nodiscard]] bool isFinished() const;
        [[nodiscard]] const std::vector<SessionToken>& getSpectators() const { return this->_spectators; }
        [[nodiscard]] RoomInfo getInfo() const;

    private:
        void stream(bool enabled);

        RoomId _id;
        std::string _name;
        std::vector<SessionToken> _spectators;
        std::shared_ptr<GameInstance> _game;
        std::uint8_t _maxSpectators;
        OutgoingSink _sink;
};

} // namespace server

#endif // GAMEENGINE_SERVER_LOBBY_ROOM_HPP
