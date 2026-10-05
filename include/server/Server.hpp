/**
 * @file Server.hpp
 * @brief server class 
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <atomic>
#include <asio.hpp>
#include "common/ServerConfig.hpp"
#include "lobby/SessionRegistry.hpp"
#include "lobby/RoomManager.hpp"
#include "net/TcpServer.hpp"
#include "net/UdpServer.hpp"

namespace server {

class Server {
    public:
        explicit Server(ServerConfig conf);

        void run();
        void stop();

    private:
        void send(OutboundMessage msg);
        void armSweepTimer();

        ServerConfig _conf;
        asio::io_context _io;
        asio::signal_set _signals;
        asio::steady_timer _sweepTimer;
        SessionRegistry _sessions;
        RoomManager _rooms;
        UdpServer _udp;
        TcpServer _tcp;
        std::atomic<bool> _stopped{false};
};

}

#endif /* SERVER_HPP_ */
