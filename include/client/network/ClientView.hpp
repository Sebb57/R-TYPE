/**
 * @file ClientView.hpp
 * @brief client side view of the lobby and game state
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_NETWORK_CLIENTVIEW_HPP
#define CLIENT_NETWORK_CLIENTVIEW_HPP

#include <cstdint>
#include <array>
#include <string>
#include <vector>

namespace client {

struct RoomView {
    std::uint32_t id = 0;
    std::string name;
    std::uint8_t players = 0;
    std::uint8_t capacity = 0;
    std::uint8_t state = 0;

    bool operator==(const RoomView&) const = default;
};

struct PlayerView {
    std::uint8_t slot = 0;
    std::string name;
    bool udpBound = false;
    bool ready = false;
};

struct EntityView {
    std::uint32_t id = 0;
    std::uint8_t type = 0;
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::uint8_t flags = 0;
};

struct CollisionEffectView {
    std::uint32_t eventId = 0;
    std::uint32_t entityId = 0;
    std::uint8_t type = 0;
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct ClientView {
    std::string status = "Not connected";
    std::vector<RoomView> rooms;
    std::vector<PlayerView> players;
    std::vector<EntityView> entities;
    std::vector<CollisionEffectView> collisionEffects;
    std::string username;
    bool connected = false;
    bool inRoom = false;
    bool running = false;
    bool host = false;
    bool udpReady = false;
    std::uint32_t selectedRoom = 0;
    std::uint32_t tick = 0;
    std::uint64_t token = 0;
    std::uint16_t udpPort = 0;
    std::uint8_t slot = 0;
    std::uint8_t snapshotRate = 20;
    std::uint16_t remainingSeconds = 180;
    bool gameOver = false;
    bool hasGameState = false;
    std::uint8_t aliveMask = 0;
    std::array<std::uint32_t, 4> scores{};
};

} // namespace client

#endif // CLIENT_NETWORK_CLIENTVIEW_HPP
