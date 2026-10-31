/**
 * @file TcpConnection.hpp
 * @brief tcp connection class
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_TCPCONNECTION_HPP_
#define GAMEENGINE_TCPCONNECTION_HPP_

#include <asio.hpp>
#include <deque>
#include "GameEngine/server/common/ServerConfig.hpp"

namespace server {

class RoomManager;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(asio::ip::tcp::socket, RoomManager&, const ServerConfig&);

        void start();
        void send(const Buffer&);
        void close();
        void left(LeaveReason reason);
        [[nodiscard]] std::optional<SessionToken> session() const { return _token; }
        [[nodiscard]] const std::string& username() const { return _username; }
        void setSession(SessionToken token) { this->_token = token; }

    private:
        void readFrameLength();
        void readFrameBody(std::uint16_t len);
        void onFrame(const Buffer& frame);
        void flushWrites();

        enum class State { AwaitingConnect, Lobby, Spectating };

        asio::ip::tcp::socket _socket;
        asio::steady_timer _deadline;
        State _state = State::AwaitingConnect;
        std::string _username;
        std::optional<SessionToken> _token;
        std::deque<Buffer> _writeQueue;
        bool _closeAfterWrite = false;
        RoomManager& _rooms;
        ServerConfig _conf;
};

} // namespace server

#endif /* GAMEENGINE_TCPCONNECTION_HPP_ */
