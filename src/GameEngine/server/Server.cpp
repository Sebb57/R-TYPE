/**
 * @file Server.cpp
 * @brief server class implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "GameEngine/server/Server.hpp"
#include <iostream>
#include <utility>

server::Server::Server(ServerConfig conf)
    : _conf(conf)
    , _work(asio::make_work_guard(this->_io))
    , _sweepTimer(this->_io)
    , _sessions()
    , _rooms(this->_sessions,
             [this](OutboundMessage&& msg) {
                 send(std::move(msg));
             },
             this->_conf)
    , _udp(this->_io, this->_sessions, this->_rooms, this->_conf)
    , _tcp(this->_io, this->_rooms, this->_conf)
{}

server::Server::~Server()
{
    this->stop();
}

void server::Server::start()
{
    if (this->_thread.joinable() || this->_stopped)
        return;
    try {
        this->_udp.start();
        this->_tcp.start();
        std::cerr << "[Server] listening tcp=" << this->_conf.tcpPort << " udp=" << this->_conf.udpPort
            << " tick=" << this->_conf.tickRate << "Hz snapshot=" << this->_conf.snapshotRate << "Hz" << std::endl;
    }
    catch (const std::exception& e) {
        this->_tcp.stop();
        this->_udp.stop();
        std::cerr << "[Server] network unavailable (" << e.what() << "), the game is only hosted locally" << std::endl;
    }
    armSweepTimer();
    this->_thread = std::jthread([this] { this->run(); });
}

void server::Server::stop()
{
    bool expected = false;
    if (!this->_stopped.compare_exchange_strong(expected, true))
        return;

    asio::post(this->_io, [this] {
        this->_rooms.stopAll();
        this->_tcp.stop();
        this->_udp.stop();
        asio::error_code ec;
        this->_sweepTimer.cancel(ec);
        this->_work.reset();
        this->_io.stop();
    });
    if (this->_thread.joinable())
        this->_thread.join();
}

void server::Server::host(std::string username, OutgoingSink local)
{
    asio::post(this->_io, [this, username = std::move(username), local = std::move(local)]() mutable {
        if (this->_host)
            this->_rooms.leave(*this->_host, LeaveReason::Requested);
        this->_host.reset();
        auto result = this->_rooms.host(std::move(username), std::move(local));
        if (!result) {
            std::cerr << "[Server] cannot host a game, error=" << static_cast<unsigned>(result.error()) << std::endl;
            return;
        }
        this->_host = result->token;
    });
}

void server::Server::leave()
{
    asio::post(this->_io, [this] {
        if (this->_host)
            this->_rooms.leave(*this->_host, LeaveReason::Requested);
        this->_host.reset();
    });
}

void server::Server::setKeys(std::uint8_t keys)
{
    asio::post(this->_io, [this, keys] {
        const Session* session = this->_host ? this->_sessions.find(*this->_host) : nullptr;
        if (session != nullptr)
            this->_rooms.push(session->room, PlayerInput(session->slot, keys));
    });
}

void server::Server::run()
{
    while (!this->_io.stopped()) {
        try {
            this->_io.run();
            break;
        }
        catch (const std::exception& e) {
            std::cerr << "[Server] exception: " << e.what() << std::endl;
        }
    }
}

void server::Server::send(OutboundMessage msg)
{
    asio::post(
        this->_io,
        [this, msg = std::move(msg)]() mutable {
            this->_udp.send(msg);
        }
    );
}

void server::Server::armSweepTimer()
{
    this->_sweepTimer.expires_after(std::chrono::seconds(1));
    this->_sweepTimer.async_wait(
        [this](const asio::error_code& ec) {
            if (ec)
                return;
            this->_rooms.sweep(Clock::now());
            if (!this->_stopped)
                armSweepTimer();
        }
    );
}
