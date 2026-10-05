/**
 * @file Server.cpp
 * @brief server class implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "Server.hpp"
#include <iostream>
#include <utility>

server::Server::Server(ServerConfig conf)
    : _conf(conf)
    , _signals(this->_io, SIGINT, SIGTERM)
    , _sweepTimer(this->_io)
    , _sessions()
    , _rooms(this->_sessions,
             [this](OutboundMessage&& msg) {
                 send(std::move(msg));
             },
             this->_conf)
    , _udp(this->_io, this->_sessions, this->_rooms, this->_conf)
    , _tcp(this->_io, this->_rooms, this->_conf)
{
    this->_signals.async_wait(
        [this](const asio::error_code& ec, int /*signal*/) {
            if (!ec)
                stop();
        }
    );
}

void server::Server::run()
{
    this->_udp.start();
    this->_tcp.start();
    armSweepTimer();

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

void server::Server::stop()
{
    bool expected = false;
    if (!this->_stopped.compare_exchange_strong(expected, true))
        return;

    this->_rooms.stopAll();
    this->_tcp.stop();
    this->_udp.stop();
    asio::error_code ec;
    this->_sweepTimer.cancel(ec);
    this->_signals.cancel(ec);
    this->_io.stop();
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
