/**
 * @file UdpProtocol.hpp
 * @brief UDP protocol definitions
 *
 * @author Seb
 * @date 2026-10-12
**/

#ifndef GAMEENGINE_PROTOCOL_UDPPROTOCOL_HPP
#define GAMEENGINE_PROTOCOL_UDPPROTOCOL_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include "GameEngine/server/common/ServerConfig.hpp"

namespace server {

constexpr std::size_t UdpHeaderSize = 18;
constexpr std::size_t MaxUdpDatagram = 1200;
constexpr std::uint16_t UdpMagic = 0x6767;
constexpr std::uint8_t UdpVersion = 2;

constexpr std::size_t SnapshotHeaderSize = 8;
constexpr std::size_t SnapshotEntitySize = 19;
constexpr std::size_t MaxImagePath = 255;
constexpr std::uint8_t NoImage = 0xff;

enum class UdpType : std::uint8_t { Hello = 1, HelloAck, Snapshot, Ping, Pong };

struct UdpHeader {
    std::uint16_t magic = UdpMagic;
    std::uint8_t version = UdpVersion;
    UdpType type = UdpType::Hello;
    std::uint16_t payloadSize = 0;
    std::uint32_t sequence = 0;
    SessionToken token = 0;
};

struct UdpProtocol {
    static std::optional<UdpHeader> parseHeader(const Buffer&, std::size_t);
    static Buffer encode(UdpType, std::uint32_t sequence, SessionToken, const Buffer& payload = {});
    static bool readU32(const Buffer&, std::size_t, std::uint32_t&);
    static void appendU32(Buffer&, std::uint32_t);
    static void appendU64(Buffer&, std::uint64_t);
};

} // namespace server

#endif
