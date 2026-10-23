/**
 * @file Server.hpp
 * @brief server embedded in the client, lets spectators watch the local solo game
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_SERVER_HPP
#define GAMEENGINE_SERVER_SERVER_HPP

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <asio.hpp>
#include "GameEngine/server/common/ServerConfig.hpp"
#include "GameEngine/server/common/ServerMessages.hpp"
#include "GameEngine/server/game/GameInstance.hpp"
#include "GameEngine/server/lobby/SessionRegistry.hpp"
#include "GameEngine/server/lobby/RoomManager.hpp"
#include "GameEngine/server/net/TcpServer.hpp"
#include "GameEngine/server/net/UdpServer.hpp"

namespace server {

class Server {
    public:
        explicit Server(ServerConfig conf);
        ~Server();

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&&) = delete;
        Server& operator=(Server&&) = delete;

        void start();
        void stop();
        [[nodiscard]] const ServerConfig& config() const { return this->_conf; }

        void host(std::string username, std::shared_ptr<GameInstance> game);
        void leave();

    private:
        void run();
        void send(OutboundMessage msg);
        void armSweepTimer();

        ServerConfig _conf;
        asio::io_context _io;
        asio::executor_work_guard<asio::io_context::executor_type> _work;
        asio::steady_timer _sweepTimer;
        SessionRegistry _sessions;
        RoomManager _rooms;
        UdpServer _udp;
        TcpServer _tcp;
        std::optional<RoomId> _hosted;
        std::atomic<bool> _stopped{false};
        std::jthread _thread;
};

} // namespace server

#endif // GAMEENGINE_SERVER_SERVER_HPP
