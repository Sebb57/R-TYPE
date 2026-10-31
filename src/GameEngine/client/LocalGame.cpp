/**
 * @file LocalGame.cpp
 * @brief solo game computed and rendered locally implementation
 *
 * @author Seb
 * @date 2026-10-22
**/

#include "GameEngine/client/LocalGame.hpp"
#include <utility>

client::LocalGame::LocalGame(server::Server& server, OrByte::IGraphic& graphic, server::GameInstance::Setup setup)
    : _server(server)
    , _graphic(graphic)
    , _setup(std::move(setup))
{}

client::LocalGame::~LocalGame()
{
    this->leave();
}

void client::LocalGame::start(const std::string& username)
{
    this->leave();
    const server::ServerConfig& config = this->_server.config();
    this->_game = std::make_shared<server::GameInstance>(this->_graphic, config.tickRate, config.snapshotRate, this->_setup);
    this->_game->start();
    this->_server.host(username, this->_game);
}

void client::LocalGame::leave()
{
    if (!this->_game)
        return;
    this->_server.leave();
    this->_game->stop();
    this->_game.reset();
}

void client::LocalGame::push(const OrByte::Event& event)
{
    if (this->_game)
        this->_game->push(event);
}

void client::LocalGame::render()
{
    if (this->_game)
        this->_game->render();
}

bool client::LocalGame::running() const
{
    return this->_game && !this->_game->finished();
}
