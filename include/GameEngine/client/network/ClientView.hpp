/**
 * @file ClientView.hpp
 * @brief client side view of the browsed server and the spectated game
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_NETWORK_CLIENTVIEW_HPP
#define GAMEENGINE_CLIENT_NETWORK_CLIENTVIEW_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "GameEngine/ECS/Utils.hpp"
#include "plugins/IGraphic.hpp"

namespace client {

struct RoomView {
    std::uint32_t id = 0;
    std::string name;
    std::uint8_t spectators = 0;
    std::uint8_t capacity = 0;

    bool operator==(const RoomView&) const = default;
};

struct EntityView {
    std::uint32_t id = 0;
    Vector2 position;
    float rotation = 0.0f;
    Vector2 size;
    OrByte::RGBA color;
    std::string image;
};

struct ClientView {
    std::string status = "Not connected";
    std::vector<RoomView> rooms;
    std::vector<EntityView> entities;
    std::string username;
    bool connected = false;
    bool spectating = false;
    bool udpReady = false;
    std::uint32_t room = 0;
    std::uint32_t tick = 0;
    std::uint64_t token = 0;
    std::uint16_t udpPort = 0;
    std::uint8_t snapshotRate = 60;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_NETWORK_CLIENTVIEW_HPP
