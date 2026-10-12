/**
 * @file UdpServer.hpp
 * @brief UDP gameplay transport
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_NET_UDPSERVER_HPP
#define SERVER_NET_UDPSERVER_HPP

#include <asio.hpp>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include "server/common/ServerConfig.hpp"
#include "server/common/ServerMessages.hpp"
#include "server/lobby/SessionRegistry.hpp"
#include "protocol/UdpProtocol.hpp"

namespace server {

class RoomManager;

class UdpServer {
    public:
        UdpServer(asio::io_context&, SessionRegistry&, RoomManager&, const ServerConfig&);

        void start();
        void stop();
        void send(const OutboundMessage&);

    private:
        void doReceive();
        void onDatagram(const asio::ip::udp::endpoint&, std::size_t);
        void sendRaw(const asio::ip::udp::endpoint&, std::shared_ptr<const Buffer>);
        void onHello(Session&, const asio::ip::udp::endpoint&);
        void onInput(Session&, const UdpHeader&, std::size_t);
        void onPing(Session&, const UdpHeader&, std::size_t);

        asio::ip::udp::socket _socket;
        Buffer _rx = Buffer(1500);
        asio::ip::udp::endpoint _sender;
        SessionRegistry& _sessions;
        RoomManager& _rooms;
        std::uint16_t _port;
        std::uint8_t _tickRate;
        std::uint8_t _snapshotRate;
        std::unordered_map<RoomId, std::size_t> _lastSnapshotCounts;
};

} // namespace server

#endif // SERVER_NET_UDPSERVER_HPP
