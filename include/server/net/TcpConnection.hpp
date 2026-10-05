/**
 * @file TcpConnection.hpp
 * @brief tcp connection class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef TCPCONNECTION_HPP_
#define TCPCONNECTION_HPP_

#include <asio.hpp>
#include <deque>
#include "server/common/ServerConfig.hpp"

namespace server {

class RoomManager;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(asio::ip::tcp::socket, RoomManager&, const ServerConfig&);

        void start();
        void send(const Buffer&);
        void close();
        std::optional<SessionToken> session() const;
        [[nodiscard]] const std::string& username() const { return _username; }
        void setSession(SessionToken token) { _token = token; }

    private:
        void readFrameLength();
        void readFrameBody(std::uint16_t len);
        void onFrame(const Buffer& frame);
        void flushWrites();

        enum class State { AwaitingConnect, Lobby, InRoom };

        asio::ip::tcp::socket _socket;
        asio::steady_timer _deadline;
        State _state = State::AwaitingConnect;
        std::string _username;
        std::optional<SessionToken> _token;
        std::deque<Buffer> _writeQueue;
        RoomManager& _rooms;
        ServerConfig _conf;
};

} // namespace server

#endif /* TCPCONNECTION_HPP_ */
