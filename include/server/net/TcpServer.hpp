/**
 * @file TcpServer.hpp
 * @brief TCP lobby listener
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_NET_TCPSERVER_HPP
#define SERVER_NET_TCPSERVER_HPP

#include <asio.hpp>
#include <memory>
#include <vector>
#include "server/common/ServerConfig.hpp"

namespace server {

class RoomManager;
class TcpConnection;

class TcpServer {
    public:
        TcpServer(asio::io_context&, RoomManager&, const ServerConfig&);

        void start();
        void stop();

    private:
        void doAccept();

        asio::ip::tcp::acceptor _acceptor;
        RoomManager& _rooms;
        ServerConfig _conf;
        std::vector<std::weak_ptr<TcpConnection>> _live;
};

} // namespace server

#endif // SERVER_NET_TCPSERVER_HPP
