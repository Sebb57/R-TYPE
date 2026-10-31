/**
 * @file RoomListScene.cpp
 * @brief game list menu implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/scenes/RoomListScene.hpp"
#include <string>
#include "GameEngine/client/network/NetworkClient.hpp"
#include "GameEngine/client/scenes/SceneManager.hpp"
#include "GameEngine/client/scenes/ServerScene.hpp"
#include "GameEngine/client/scenes/SpectateScene.hpp"

namespace {

std::string describe(const client::RoomView& room)
{
    return std::to_string(room.spectators) + "/" + std::to_string(room.capacity) + "    watching";
}

} // namespace

client::RoomListScene::RoomListScene(SceneContext& context) : AMenuScene(context)
{
    this->_subtitle = "GAMES";
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
    if (this->_view.spectating) {
        this->_context.scenes.switchTo(SpectateScene::Name);
        return;
    }
    this->_subtitle = "GAMES    browsing as " + this->_view.username;
    if (this->_view.rooms == this->_rooms)
        return;
    this->_rooms = this->_view.rooms;
    this->rebuild();
}

void client::RoomListScene::rebuild()
{
    this->clearEntries();
    for (const RoomView& room : this->_rooms) {
        const std::uint32_t id = room.id;
        const bool open = room.spectators < room.capacity;
        this->addButton(room.name + "   #" + std::to_string(id), [this, id] { this->_context.network.spectate(id); }, [open] { return open; }, describe(room));
    }
    this->addButton("Refresh", [this] { this->_context.network.listRooms(); });
    this->addButton("Disconnect", [this] { this->_context.network.disconnect(); });
}
