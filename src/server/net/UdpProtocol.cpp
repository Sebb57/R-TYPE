/**
 * @file UdpProtocol.cpp
 * @brief udp protocol implementation
 *
 * @author Seb
 * @date 2026-10-12
**/

#include "protocol/UdpProtocol.hpp"

namespace server {

std::optional<UdpHeader> UdpProtocol::parseHeader(const Buffer& buffer, std::size_t size)
{
    if (size < UdpHeaderSize || size > MaxUdpDatagram || size > buffer.size())
        return std::nullopt;
    UdpHeader header;
    header.magic = static_cast<std::uint16_t>(buffer[0]) | static_cast<std::uint16_t>(buffer[1] << 8);
    header.version = buffer[2];
    const auto rawType = buffer[3];
    if (rawType < static_cast<Byte>(UdpType::Hello) || rawType > static_cast<Byte>(UdpType::Disconnect))
        return std::nullopt;
    header.type = static_cast<UdpType>(rawType);
    header.payloadSize = static_cast<std::uint16_t>(buffer[4]) | static_cast<std::uint16_t>(buffer[5] << 8);
    for (unsigned shift = 0; shift < 32; shift += 8)
        header.sequence |= static_cast<std::uint32_t>(buffer[6 + shift / 8]) << shift;
    for (unsigned shift = 0; shift < 64; shift += 8)
        header.token |= static_cast<std::uint64_t>(buffer[10 + shift / 8]) << shift;
    if (header.magic != UdpMagic || header.version != UdpVersion || header.payloadSize != size - UdpHeaderSize)
        return std::nullopt;
    return header;
}

Buffer UdpProtocol::encode(UdpType type, std::uint32_t sequence, SessionToken token, const Buffer& payload)
{
    if (payload.size() > MaxUdpDatagram - UdpHeaderSize)
        return {};
    Buffer result;
    result.reserve(UdpHeaderSize + payload.size());
    result.push_back(static_cast<Byte>(UdpMagic));
    result.push_back(static_cast<Byte>(UdpMagic >> 8));
    result.push_back(UdpVersion);
    result.push_back(static_cast<Byte>(type));
    result.push_back(static_cast<Byte>(payload.size()));
    result.push_back(static_cast<Byte>(payload.size() >> 8));
    appendU32(result, sequence);
    appendU64(result, token);
    result.insert(result.end(), payload.begin(), payload.end());
    return result;
}

bool UdpProtocol::readU32(const Buffer& buffer, std::size_t offset, std::uint32_t& value)
{
    if (offset > buffer.size() || buffer.size() - offset != 4)
        return false;
    value = 0;
    for (unsigned shift = 0; shift < 32; shift += 8)
        value |= static_cast<std::uint32_t>(buffer[offset++]) << shift;
    return true;
}

void UdpProtocol::appendU32(Buffer& buffer, std::uint32_t value)
{
    for (unsigned shift = 0; shift < 32; shift += 8)
        buffer.push_back(static_cast<Byte>(value >> shift));
}

void UdpProtocol::appendU64(Buffer& buffer, std::uint64_t value)
{
    for (unsigned shift = 0; shift < 64; shift += 8)
        buffer.push_back(static_cast<Byte>(value >> shift));
}

} // namespace server
