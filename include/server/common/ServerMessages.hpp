/**
 * @file ServerMessages.hpp
 * @brief server-side messages passed between lobby, game and network
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVERMESSAGES_HPP_
#define SERVERMESSAGES_HPP_

#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>
#include "ServerConfig.hpp"

namespace server {

struct RoomInfo {
    RoomId id;
    std::string name;
    std::uint8_t players;
    std::uint8_t maxPlayers;
    RoomState state;
};

struct PlayerJoined { PlayerSlot slot; std::string name; };
struct PlayerLeft { PlayerSlot slot; LeaveReason reason; };
struct PlayerInput { PlayerSlot slot; std::uint8_t keys; };
using InboundEvent = std::variant<PlayerJoined, PlayerLeft, PlayerInput>;

struct Target {
    enum class Kind { All, Only, AllExcept } kind;
    PlayerSlot slot = 0;
};

struct SnapshotDto { std::vector<Byte> data; };
struct GameEventDto { std::vector<Byte> data; };
struct OutboundMessage {
    RoomId room;
    Target target;
    std::variant<SnapshotDto, GameEventDto> payload;
};
using OutgoingSink = std::function<void(OutboundMessage&&)>;

}

#endif /* SERVERMESSAGES_HPP_ */
