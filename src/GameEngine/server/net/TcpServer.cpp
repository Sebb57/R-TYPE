/**
 * @file TcpServer.cpp
 * @brief tcp server implementation
 *
 * @author Seb
 * @date 2026-10-08
**/

#include <algorithm>
#include "GameEngine/server/lobby/RoomManager.hpp"
#include "GameEngine/server/net/TcpConnection.hpp"
#include "GameEngine/server/net/TcpServer.hpp"

server::TcpServer::TcpServer(asio::io_context& io, RoomManager& rooms, const ServerConfig& conf)
    : _acceptor(io)
    , _rooms(rooms)
    , _conf(conf)
{}

void server::TcpServer::start()
{
    const asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), this->_conf.tcpPort);
    this->_acceptor.open(endpoint.protocol());
    this->_acceptor.set_option(asio::socket_base::reuse_address(true));
    this->_acceptor.bind(endpoint);
    this->_acceptor.listen(asio::socket_base::max_listen_connections);
    this->doAccept();
}

void server::TcpServer::stop()
{
    asio::error_code ignored;
    this->_acceptor.cancel(ignored);
    this->_acceptor.close(ignored);
    for (auto& weak : this->_live) {
        if (auto connection = weak.lock())
            connection->close();
    }
    this->_live.clear();
}

void server::TcpServer::doAccept()
{
    this->_acceptor.async_accept([this](const asio::error_code& error, asio::ip::tcp::socket socket) {
        if (!error) {
            std::erase_if(this->_live, [](const auto& item) { return item.expired(); });
            if (this->_live.size() < this->_conf.maxTcpConnections) {
                auto connection = std::make_shared<TcpConnection>(std::move(socket), this->_rooms, this->_conf);
                this->_live.emplace_back(connection);
                connection->start();
            } else {
                asio::error_code ignored;
                socket.close(ignored);
            }
        }
        if (this->_acceptor.is_open())
            this->doAccept();
    });
}
