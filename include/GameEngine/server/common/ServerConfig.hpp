/**
 * @file ServerConfig.hpp
 * @brief server config class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_COMMON_SERVERCONFIG_HPP
#define GAMEENGINE_SERVER_COMMON_SERVERCONFIG_HPP

#include <cstdint>
#include <chrono>
#include <expected>
#include <cstddef>
#include <vector>

namespace server {

using RoomId = std::uint32_t;
using SessionToken = std::uint64_t;
using Clock = std::chrono::steady_clock;
using Byte = std::uint8_t;
using Buffer = std::vector<Byte>;

enum class LeaveReason : uint8_t { Requested, ConnectionLost, Timeout, GameEnded };
enum class LobbyError : uint8_t { BadRequest, BadVersion, BadName, RoomNotFound, RoomFull, TooManyRooms };

template <class T> using Result = std::expected<T, LobbyError>;

struct ServerConfig {
    std::uint16_t tcpPort = 6969, udpPort = 6767;
    std::size_t maxRooms = 16, maxTcpConnections = 64;
    std::uint8_t maxSpectatorsPerRoom = 8;
    unsigned tickRate = 60, snapshotRate = 60;
    std::chrono::seconds clientTimeout{5};
    std::chrono::seconds handshakeTimeout{5};
};

} // namespace server

#endif // GAMEENGINE_SERVER_COMMON_SERVERCONFIG_HPP
