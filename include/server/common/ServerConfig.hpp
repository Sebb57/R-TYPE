/**
 * @file ServerConfig.hpp
 * @brief server config class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_COMMON_SERVERCONFIG_HPP
#define SERVER_COMMON_SERVERCONFIG_HPP

#include <cstdint>
#include <chrono>
#include <expected>
#include <cstddef>
#include <string>
#include <vector>

namespace server {

using RoomId = std::uint32_t;
using SessionToken = std::uint64_t; // 0 = invalide
using PlayerSlot = std::uint8_t;
using Clock = std::chrono::steady_clock;
using Byte = std::uint8_t;
using Buffer = std::vector<Byte>;

enum class RoomState   { Lobby, Running, Finished };
enum class LeaveReason { Requested, ConnectionLost, Timeout, Kicked };
enum class LobbyError  { BadName, RoomFull, RoomNotFound, AlreadyStarted,
                         NotHost, PlayersNotReady, TooManyRooms, NotInRoom };

template <class T> using Result = std::expected<T, LobbyError>;

struct ServerConfig {
    std::uint16_t tcpPort = 6969, udpPort = 6767;
    std::size_t maxRooms = 16, maxTcpConnections = 64;
    std::uint8_t maxPlayersPerRoom = 4;
    unsigned tickRate = 60, snapshotRate = 20;
    std::chrono::seconds clientTimeout{5};
    std::chrono::seconds handshakeTimeout{5};
};

} // namespace server

#endif // SERVER_COMMON_SERVERCONFIG_HPP
