/**
 * @file RoomListScene.cpp
 * @brief room list menu implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "client/scenes/RoomListScene.hpp"
#include <array>
#include <string>
#include "client/network/NetworkClient.hpp"
#include "client/scenes/RoomScene.hpp"
#include "client/scenes/SceneManager.hpp"
#include "client/scenes/ServerScene.hpp"
#include "protocol/UdpProtocol.hpp"

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
    if (this->_view.inRoom) {
        this->_context.scenes.switchTo(RoomScene::Name);
        return;
    }
    this->_subtitle = "ROOMS    playing as " + this->_view.username;
    if (this->_view.rooms == this->_rooms)
        return;
    this->_rooms = this->_view.rooms;
    this->rebuild();
}

void client::RoomListScene::rebuild()
{
    this->clearEntries();
    this->addButton("Create room", [this] { this->_context.network.createRoom(); });
    for (const RoomView& room : this->_rooms) {
        const std::uint32_t id = room.id;
        const bool open = room.players < room.capacity && room.state != static_cast<std::uint8_t>(server::RoomState::Finished);
        this->addButton(room.name + "   #" + std::to_string(id), [this, id] { this->_context.network.joinRoom(id); }, [open] { return open; }, describe(room));
    }
    this->addButton("Refresh", [this] { this->_context.network.listRooms(); });
    this->addButton("Disconnect", [this] { this->_context.network.disconnect(); });
}
