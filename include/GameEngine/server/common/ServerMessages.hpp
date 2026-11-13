/**
 * @file ServerMessages.hpp
 * @brief server-side messages passed between lobby, game and network
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_COMMON_SERVERMESSAGES_HPP
#define GAMEENGINE_SERVER_COMMON_SERVERMESSAGES_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "GameEngine/ECS/Utils.hpp"
#include "GameEngine/server/common/ServerConfig.hpp"
#include "plugins/IGraphic.hpp"

namespace server {

struct RoomInfo {
    RoomId id;
    std::string name;
    std::uint8_t spectators;
    std::uint8_t maxSpectators;
};

struct EntityDto {
    std::uint32_t id = 0;
    Vector2 position;
    float rotation = 0.0f;
    Vector2 size;
    OrByte::RGBA color;
    std::string image;
    Rect frame;
};
struct SnapshotDto {
    std::uint32_t tick = 0;
    std::vector<EntityDto> entities;
};
struct OutboundMessage {
    RoomId room;
    SnapshotDto snapshot;
};
using SnapshotSink = std::function<void(SnapshotDto&&)>;
using OutgoingSink = std::function<void(OutboundMessage&&)>;

} // namespace server

#endif // GAMEENGINE_SERVER_COMMON_SERVERMESSAGES_HPP
