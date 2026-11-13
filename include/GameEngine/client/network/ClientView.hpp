/**
 * @file ClientView.hpp
 * @brief client side view of the browsed server and the spectated game
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_NETWORK_CLIENTVIEW_HPP
#define GAMEENGINE_CLIENT_NETWORK_CLIENTVIEW_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "GameEngine/ECS/Utils.hpp"
#include "plugins/IGraphic.hpp"
#ifdef ORBYTE_FEATURE_CHAT
    #include "GameEngine/protocol/Chat.hpp"
#endif

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

#ifdef ORBYTE_FEATURE_LAGOMETER
struct NetworkMetrics {
    static constexpr std::size_t History = 120;
    static constexpr float Lost = -1.0F;

    std::vector<float> samples;
    float last = 0.0F;
    float smoothed = 0.0F;
    float jitter = 0.0F;
    std::uint32_t sent = 0;
    std::uint32_t received = 0;
    std::uint32_t lost = 0;
};
#endif

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
#ifdef ORBYTE_FEATURE_LAGOMETER
    NetworkMetrics metrics;
#endif
#ifdef ORBYTE_FEATURE_CHAT
    bool chatAvailable = false;
    server::ChatLog chat;
#endif
};

} // namespace client

#endif // GAMEENGINE_CLIENT_NETWORK_CLIENTVIEW_HPP
