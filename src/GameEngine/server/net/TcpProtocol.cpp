/**
 * @file TcpProtocol.cpp
 * @brief tcp server implementation
 *
 * @author Seb
 * @date 2026-10-12
**/

#include "GameEngine/protocol/TcpProtocol.hpp"
#include <cstddef>

void server::TcpProtocol::appendU16(Buffer& buffer, std::uint16_t value)
{
    buffer.push_back(static_cast<Byte>(value));
    buffer.push_back(static_cast<Byte>(value >> 8));
}

void server::TcpProtocol::appendU32(Buffer& buffer, std::uint32_t value)
{
    for (unsigned shift = 0; shift < 32; shift += 8)
        buffer.push_back(static_cast<Byte>(value >> shift));
}

void server::TcpProtocol::appendU64(Buffer& buffer, std::uint64_t value)
{
    for (unsigned shift = 0; shift < 64; shift += 8)
        buffer.push_back(static_cast<Byte>(value >> shift));
}

void server::TcpProtocol::appendString(Buffer& buffer, const std::string& value)
{
    if (value.size() > 255)
        return;
    buffer.push_back(static_cast<Byte>(value.size()));
    buffer.insert(buffer.end(), value.begin(), value.end());
}

bool server::TcpProtocol::readU8(const Buffer& buffer, std::size_t& offset, std::uint8_t& value)
{
    if (offset >= buffer.size())
        return false;
    value = buffer[offset++];
    return true;
}

bool server::TcpProtocol::readU16(const Buffer& buffer, std::size_t& offset, std::uint16_t& value)
{
    if (offset > buffer.size() || buffer.size() - offset < 2)
        return false;
    value = static_cast<std::uint16_t>(buffer[offset]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(buffer[offset + 1]) << 8);
    offset += 2;
    return true;
}

bool server::TcpProtocol::readU32(const Buffer& buffer, std::size_t& offset, std::uint32_t& value)
{
    if (offset > buffer.size() || buffer.size() - offset < 4)
        return false;
    value = 0;
    for (unsigned shift = 0; shift < 32; shift += 8)
        value |= static_cast<std::uint32_t>(buffer[offset++]) << shift;
    return true;
}

bool server::TcpProtocol::readU64(const Buffer& buffer, std::size_t& offset, std::uint64_t& value)
{
    if (offset > buffer.size() || buffer.size() - offset < 8)
        return false;
    value = 0;
    for (unsigned shift = 0; shift < 64; shift += 8)
        value |= static_cast<std::uint64_t>(buffer[offset++]) << shift;
    return true;
}

bool server::TcpProtocol::readString(const Buffer& buffer, std::size_t& offset, std::size_t maximum, std::string& value)
{
    std::uint8_t length = 0;
    if (!readU8(buffer, offset, length) || length > maximum || offset > buffer.size() || buffer.size() - offset < length)
        return false;
    value.assign(buffer.begin() + static_cast<std::ptrdiff_t>(offset),
        buffer.begin() + static_cast<std::ptrdiff_t>(offset + length));
    offset += length;
    return true;
}

server::Buffer server::TcpProtocol::message(Type type, Buffer payload)
{
    Buffer result{static_cast<Byte>(type)};
    result.insert(result.end(), payload.begin(), payload.end());
    return result;
}

server::Buffer server::TcpProtocol::frame(const Buffer& message)
{
    if (message.empty() || message.size() > MaxFrameSize)
        return {};
    Buffer result;
    result.reserve(message.size() + 2);
    appendU16(result, static_cast<std::uint16_t>(message.size()));
    result.insert(result.end(), message.begin(), message.end());
    return result;
}
