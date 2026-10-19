/**
 * @file ServerMessages.hpp
 * @brief server-side messages passed between lobby, game and network
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_COMMON_SERVERMESSAGES_HPP
#define SERVER_COMMON_SERVERMESSAGES_HPP

#include <cstdint>
#include <array>
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

struct EntityDto { std::uint32_t id = 0; std::uint8_t type = 0; std::int16_t x = 0; std::int16_t y = 0; std::uint8_t flags = 0; };
struct SnapshotDto {
    std::uint32_t tick = 0;
    std::uint16_t remainingSeconds = 180;
    bool gameOver = false;
    std::uint8_t aliveMask = 0;
    std::array<std::uint32_t, 4> scores{};
    std::vector<EntityDto> entities;
};
struct GameEventDto { std::uint32_t eventId = 0; std::uint8_t eventType = 0; std::uint32_t entityId = 0; std::uint32_t extra = 0; };
struct OutboundMessage {
    RoomId room;
    Target target;
    std::variant<SnapshotDto, GameEventDto> payload;
};
using OutgoingSink = std::function<void(OutboundMessage&&)>;

} // namespace server

#endif // SERVER_COMMON_SERVERMESSAGES_HPP
