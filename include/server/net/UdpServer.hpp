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
#include <memory>
#include "server/common/ServerConfig.hpp"
#include "server/common/ServerMessages.hpp"
#include "server/lobby/SessionRegistry.hpp"

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

        asio::ip::udp::socket _socket;
        Buffer _rx = Buffer(1500);
        asio::ip::udp::endpoint _sender;
        SessionRegistry& _sessions;
        RoomManager& _rooms;
};

} // namespace server

#endif // SERVER_NET_UDPSERVER_HPP
