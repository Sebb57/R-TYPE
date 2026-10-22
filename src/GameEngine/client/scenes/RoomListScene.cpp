/**
 * @file RoomListScene.cpp
 * @brief room list menu implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/scenes/RoomListScene.hpp"
#include <array>
#include <string>
#include "GameEngine/client/network/NetworkClient.hpp"
#include "GameEngine/client/scenes/SceneManager.hpp"
#include "GameEngine/client/scenes/ServerScene.hpp"

namespace {

const std::array<std::string, 3> RoomStates{"LOBBY", "RUNNING", "FINISHED"};

std::string describe(const client::RoomView& room)
{
    const std::string state = room.state < RoomStates.size() ? RoomStates[room.state] : "?";
    return std::to_string(room.players) + "/" + std::to_string(room.capacity) + "    " + state;
}

} // namespace

client::RoomListScene::RoomListScene(SceneContext& context) : AMenuScene(context)
{
    this->_subtitle = "ROOMS";
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.network.disconnect();
    });
    this->rebuild();
    this->_context.network.listRooms();
}

void client::RoomListScene::update(double deltaTime)
{
    AMenuScene::update(deltaTime);
    if (!this->_view.connected) {
        this->_context.scenes.switchTo(ServerScene::Name);
        return;
    }
    this->_subtitle = "ROOMS    browsing as " + this->_view.username;
    if (this->_view.rooms == this->_rooms)
        return;
    this->_rooms = this->_view.rooms;
    this->rebuild();
}

void client::RoomListScene::rebuild()
{
    this->clearEntries();
    for (const RoomView& room : this->_rooms)
        this->addButton(room.name + "   #" + std::to_string(room.id), nullptr, nullptr, describe(room));
    this->addButton("Refresh", [this] { this->_context.network.listRooms(); });
    this->addButton("Disconnect", [this] { this->_context.network.disconnect(); });
}
