/**
 * @file RoomScene.cpp
 * @brief room menu implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "client/scenes/RoomScene.hpp"
#include <algorithm>
#include <string>
#include "client/network/NetworkClient.hpp"
#include "client/scenes/GameScene.hpp"
#include "client/scenes/RoomListScene.hpp"
#include "client/scenes/SceneManager.hpp"
#include "client/scenes/ServerScene.hpp"
#include "client/scenes/Theme.hpp"

namespace {

constexpr float PlayersY = 250.0f;
constexpr float PlayerHeight = 72.0f;
constexpr std::size_t MaxPlayers = 4;

} // namespace

client::RoomScene::RoomScene(SceneContext& context) : AMenuScene(context)
{
    this->_top = PlayersY + PlayerHeight * MaxPlayers + 60.0f;
    this->addButton("Start game", [this] { this->_context.network.startGame(); }, [this] { return this->canStart(); });
    this->addButton("Leave room", [this] { this->_context.network.leaveRoom(); });
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.network.leaveRoom();
    });
}

void client::RoomScene::update(double deltaTime)
{
    AMenuScene::update(deltaTime);
    if (!this->_view.connected)
        this->_context.scenes.switchTo(ServerScene::Name);
    else if (!this->_view.inRoom)
        this->_context.scenes.switchTo(RoomListScene::Name);
    else if (this->_view.running)
        this->_context.scenes.switchTo(GameScene::Name);
    const auto room = std::ranges::find(this->_view.rooms, this->_view.selectedRoom, &RoomView::id);
    this->_subtitle = "ROOM #" + std::to_string(this->_view.selectedRoom) + (room != this->_view.rooms.end() ? "    " + room->name : "") + (this->_view.host ? "    HOST" : "");
}

void client::RoomScene::render()
{
    AMenuScene::render();
    this->drawPanel(Vector2(theme::Margin, PlayersY - 20.0f), Vector2(theme::Width - 2.0f * theme::Margin, PlayerHeight * MaxPlayers + 20.0f), theme::Panel, 1, theme::PanelOutline);
    float y = PlayersY;
    for (const PlayerView& player : this->_view.players) {
        const OrByte::RGBA color = theme::Slots[player.slot % theme::Slots.size()];
        this->drawText("P" + std::to_string(player.slot + 1) + "    " + player.name + (player.slot == this->_view.slot ? "  (you)" : ""), Vector2(theme::Margin + 30.0f, y), 30, color);
        this->drawText(player.udpBound ? "READY" : "LINKING", Vector2(theme::Column, y), 30, player.udpBound ? theme::Ready : theme::Waiting);
        y += PlayerHeight;
    }
}

bool client::RoomScene::canStart() const
{
    return this->_view.host && !this->_view.running && !this->_view.players.empty() && std::ranges::all_of(this->_view.players, &PlayerView::udpBound);
}
