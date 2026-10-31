/**
 * @file SpectateScene.cpp
 * @brief spectator scene implementation
 *
 * @author Seb
 * @date 2026-10-23
**/

#include "GameEngine/client/scenes/SpectateScene.hpp"
#include <algorithm>
#include <unordered_set>
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/client/network/NetworkClient.hpp"
#include "GameEngine/client/scenes/RoomListScene.hpp"
#include "GameEngine/client/scenes/SceneManager.hpp"
#include "GameEngine/client/scenes/ServerScene.hpp"
#include "GameEngine/client/scenes/Theme.hpp"

namespace {

constexpr double StaleSnapshot = 0.25;

} // namespace

client::SpectateScene::SpectateScene(SceneContext& context) : AScene(context)
{
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.network.leave();
    });
    this->_view = this->_context.network.view();
}

void client::SpectateScene::update(double deltaTime)
{
    AScene::update(deltaTime);
    this->_view = this->_context.network.view();
    if (!this->_view.connected) {
        this->_context.scenes.switchTo(ServerScene::Name);
        return;
    }
    if (!this->_view.spectating) {
        this->_context.scenes.switchTo(RoomListScene::Name);
        return;
    }
    if (this->_view.tick != this->_tick) {
        this->_tick = this->_view.tick;
        this->_sinceSnapshot = 0.0;
        this->applySnapshot();
        return;
    }
    this->_sinceSnapshot += deltaTime;
    if (this->_sinceSnapshot >= StaleSnapshot)
        this->freeze();
}

void client::SpectateScene::render()
{
    AScene::render();
    this->drawText("SPECTATING  " + this->roomName(), Vector2(40.0F, 30.0F), 28, theme::Accent);
    if (this->_tick == 0)
        this->drawText(this->_view.status, Vector2(40.0F, 75.0F), 22, theme::Muted);
    this->drawText(this->_context.controls.keyName(Action::Back) + "  leave", Vector2(40.0F, 1020.0F), 22, theme::Muted);
}

void client::SpectateScene::applySnapshot()
{
    const auto rate = static_cast<float>(this->_view.snapshotRate);
    std::unordered_set<std::uint32_t> alive;
    for (const EntityView& remote : this->_view.entities) {
        alive.insert(remote.id);
        auto local = this->_entities.find(remote.id);
        if (local == this->_entities.end()) {
            const int entity = this->_world.createEntity();
            this->_world.addComponentToEntity<OrByte::Transform>(entity, remote.position, Vector2(1.0F, 1.0F), remote.rotation);
            this->_world.addComponentToEntity<OrByte::Velocity>(entity);
            this->_world.addComponentToEntity<OrByte::Controllable>(entity);
            local = this->_entities.emplace(remote.id, entity).first;
        }
        auto *transform = this->_world.getComponent<OrByte::Transform>(local->second);
        auto *velocity = this->_world.getComponent<OrByte::Velocity>(local->second);
        auto *controllable = this->_world.getComponent<OrByte::Controllable>(local->second);
        if (transform == nullptr || velocity == nullptr || controllable == nullptr)
            continue;
        transform->rotation = remote.rotation;
        velocity->velocity = Vector2((remote.position.x - transform->position.x) * rate, (remote.position.y - transform->position.y) * rate);
        controllable->size = remote.size;
        controllable->color = remote.color;
        controllable->image = remote.image;
    }
    for (auto entity = this->_entities.begin(); entity != this->_entities.end();) {
        if (alive.contains(entity->first)) {
            ++entity;
            continue;
        }
        this->_world.destroyEntity(entity->second);
        entity = this->_entities.erase(entity);
    }
}

void client::SpectateScene::freeze()
{
    for (const auto& [id, entity] : this->_entities)
        if (auto *velocity = this->_world.getComponent<OrByte::Velocity>(entity))
            velocity->velocity = Vector2(0.0F, 0.0F);
}

std::string client::SpectateScene::roomName() const
{
    const auto room = std::ranges::find(this->_view.rooms, this->_view.room, &RoomView::id);
    return room != this->_view.rooms.end() ? room->name : "#" + std::to_string(this->_view.room);
}
