/**
 * @file TcpProtocol.hpp
 * @brief tcp protocol
 *
 * @author Seb
 * @date 2026-10-08
**/

#ifndef TCPPROTOCOL_HPP_
#define TCPPROTOCOL_HPP_


#include <cstdint>
#include <string>
#include <cstddef>
#include <vector>
#include "server/common/ServerConfig.hpp"

namespace server {

constexpr std::uint16_t MaxFrameSize = 4096;

enum class Type : Byte { Connect = 1, ConnectOk, ListRooms, RoomList, CreateRoom, JoinRoom,
    RoomJoined, RoomUpdate, LeaveRoom, StartGame, GameStarted, Error, SetReady };

class TcpProtocol {
public:
    static void appendU16(Buffer&, std::uint16_t);
    static void appendU32(Buffer&, std::uint32_t);
    static void appendU64(Buffer&, std::uint64_t);
    static void appendString(Buffer&, const std::string&);
    static bool readU8(const Buffer&, std::size_t&, std::uint8_t&);
    static bool readU16(const Buffer&, std::size_t&, std::uint16_t&);
    static bool readU32(const Buffer&, std::size_t&, std::uint32_t&);
    static bool readU64(const Buffer&, std::size_t&, std::uint64_t&);
    static bool readString(const Buffer&, std::size_t&, std::size_t, std::string&);
    static Buffer message(Type, Buffer payload = {});
    static Buffer frame(const Buffer&);
};

} // namespace server

#endif /* TCPPROTOCOL_HPP_ */
