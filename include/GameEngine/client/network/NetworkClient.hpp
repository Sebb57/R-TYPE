/**
 * @file NetworkClient.hpp
 * @brief client network layer, tcp lobby and udp game channel
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_NETWORK_NETWORKCLIENT_HPP
#define GAMEENGINE_CLIENT_NETWORK_NETWORKCLIENT_HPP

#include <array>
#include <asio.hpp>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include "GameEngine/client/network/ClientView.hpp"
#include "GameEngine/protocol/TcpProtocol.hpp"
#include "GameEngine/protocol/UdpProtocol.hpp"

namespace client {

class NetworkClient {
    public:
        NetworkClient();
        ~NetworkClient();

        NetworkClient(const NetworkClient&) = delete;
        NetworkClient& operator=(const NetworkClient&) = delete;
        NetworkClient(NetworkClient&&) = delete;
        NetworkClient& operator=(NetworkClient&&) = delete;

        [[nodiscard]] ClientView view() const;
        void connect(std::string host, std::uint16_t port, std::string username);
        void disconnect();
        void listRooms();
        void createRoom();
        void joinRoom(std::uint32_t id);
        void leaveRoom();
        void startGame();
        void setReady(bool ready);
        void setKeys(std::uint8_t keys);

    private:
        using TcpHandler = void (NetworkClient::*)(const server::Buffer&, std::size_t);
        using UdpHandler = void (NetworkClient::*)(const server::Buffer&);

        void setStatus(std::string status);
        void sendTcp(server::Buffer message);
        void writeNext();
        void readLength();
        void readBody(std::uint16_t size);
        void onTcpMessage(const server::Buffer& message);
        void onConnectOk(const server::Buffer& message, std::size_t offset);
        void onRoomList(const server::Buffer& message, std::size_t offset);
        void onRoomJoined(const server::Buffer& message, std::size_t offset);
        void onRoomUpdate(const server::Buffer& message, std::size_t offset);
        void onGameStarted(const server::Buffer& message, std::size_t offset);
        void onError(const server::Buffer& message, std::size_t offset);
        void startUdp(std::uint16_t port, std::uint64_t token);
        void sendHello(std::uint64_t token);
        void readUdp();
        void onUdpPacket(const server::Buffer& packet);
        void onHelloAck(const server::Buffer& packet);
        void onSnapshot(const server::Buffer& packet);
        void onGameEvent(const server::Buffer& packet);
        void beginInputLoop();
        void sendInput();
        void onDisconnect(const std::string& reason);

        const std::unordered_map<server::Type, TcpHandler> _tcpHandlers;
        const std::unordered_map<server::UdpType, UdpHandler> _udpHandlers;
        asio::io_context _io;
        asio::executor_work_guard<asio::io_context::executor_type> _work;
        asio::ip::tcp::socket _tcp;
        asio::ip::udp::socket _udp;
        asio::steady_timer _helloTimer;
        asio::steady_timer _inputTimer;
        asio::ip::address _hostAddress = asio::ip::address_v4::loopback();
        asio::ip::udp::endpoint _udpEndpoint;
        asio::ip::udp::endpoint _udpSender;
        std::array<server::Byte, server::MaxUdpDatagram> _udpBuffer{};
        std::deque<std::shared_ptr<server::Buffer>> _writes;
        mutable std::mutex _mutex;
        ClientView _view;
        std::string _username;
        std::atomic<std::uint8_t> _keys{0};
        std::uint32_t _udpSequence = 0;
        std::jthread _thread;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_NETWORK_NETWORKCLIENT_HPP
