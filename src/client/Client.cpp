/**
 * @file Client.cpp
 * @brief client class implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "client/Client.hpp"
#include "constants.hpp"
#include <filesystem>
#include <iostream>
#include <queue>
#include <utility>
#include "client/scenes/GameScene.hpp"
#include "client/scenes/RoomListScene.hpp"
#include "client/scenes/RoomScene.hpp"
#include "client/scenes/ServerScene.hpp"

namespace {

std::filesystem::path findAsset(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path))
        return path;
    std::cerr << "[Client] missing asset " << path.string() << ", run the client from the repository root" << std::endl;
    return {};
}

} // namespace

client::Client::Client(ClientConfig config)
    : _config(std::move(config))
    , _graphicLoader(std::filesystem::path(OrByte::constants::PLUGINS_DIR) / OrByte::constants::pluginFile("SFML"))
    , _graphicLib(this->_graphicLoader.loadFunction<OrByte::IGraphic* (*)()>("factory")())
    , _context(this->_config, *this->_graphicLib, this->_network, this->_scenes, this->_controls, findAsset(this->_config.font))
{
    this->registerScenes();
}

void client::Client::run()
{
    this->_graphicLib->open(this->_config.width, this->_config.height, this->_config.title);
    this->_scenes.switchTo(ServerScene::Name);
    this->_clock.reset();
    while (this->_graphicLib->isOpen()) {
        this->_scenes.apply();
        this->pollEvents();
        const double deltaTime = this->_clock.getElapsedTime();
        this->_clock.reset();
        this->_scenes.update(deltaTime);
        this->_graphicLib->clear();
        this->_scenes.render();
        this->_graphicLib->update();
    }
}

void client::Client::registerScenes()
{
    this->_scenes.add(ServerScene::Name, [this] { return std::make_unique<ServerScene>(this->_context); });
    this->_scenes.add(RoomListScene::Name, [this] { return std::make_unique<RoomListScene>(this->_context); });
    this->_scenes.add(RoomScene::Name, [this] { return std::make_unique<RoomScene>(this->_context); });
    this->_scenes.add(GameScene::Name, [this] { return std::make_unique<GameScene>(this->_context); });
}

void client::Client::pollEvents()
{
    std::queue<OrByte::Event> events = this->_graphicLib->listen();
    while (!events.empty()) {
        const OrByte::Event event = events.front();
        events.pop();
        if (event.type == OrByte::Event::Type::Closed)
            this->_graphicLib->close();
        else
            this->_scenes.onEvent(event);
    }
}
