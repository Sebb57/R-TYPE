/**
 * @file GameScene.cpp
 * @brief in game scene implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "client/scenes/GameScene.hpp"
#include <filesystem>
#include <iostream>
#include <unordered_set>
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "client/network/NetworkClient.hpp"
#include "client/scenes/RoomScene.hpp"
#include "client/scenes/SceneManager.hpp"
#include "client/scenes/Theme.hpp"
#include "constants.hpp"

client::GameScene::GameScene(SceneContext& context)
    : AScene(context)
    , _inputs{
        {Action::Up, server::InputKey::Up},
        {Action::Down, server::InputKey::Down},
        {Action::Left, server::InputKey::Left},
        {Action::Right, server::InputKey::Right},
        {Action::Fire, server::InputKey::Fire},
    }
{
    for (const auto& [action, input] : this->_inputs)
        this->bindInput(action, input);
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.network.leaveRoom();
    });
    this->addSprite(server::EntityType::Player, OrByte::constants::SPRITE_PLAYER, Vector2(OrByte::constants::PLAYER_WIDTH, OrByte::constants::PLAYER_HEIGHT));
    this->addSprite(server::EntityType::Projectile, OrByte::constants::SPRITE_PROJECTILE, Vector2(OrByte::constants::PROJECTILE_WIDTH, OrByte::constants::PROJECTILE_HEIGHT));
}

client::GameScene::~GameScene()
{
    this->_context.network.setKeys(0);
}

void client::GameScene::update(double deltaTime)
{
    AScene::update(deltaTime);
    this->_view = this->_context.network.view();
    if (!this->_view.running) {
        this->_context.scenes.switchTo(RoomScene::Name);
        return;
    }
    if (this->_view.tick == this->_tick)
        return;
    this->_tick = this->_view.tick;
    this->applySnapshot(this->_view.entities);
}

void client::GameScene::render()
{
    AScene::render();
    this->drawText("Tick " + std::to_string(this->_view.tick) + "   Entities " + std::to_string(this->_entities.size()), Vector2(40.0f, 30.0f), 24, theme::Muted);
}

void client::GameScene::bindInput(Action action, server::InputKey input)
{
    this->bindAction(action, [this, input](bool pressed) {
        const auto bit = static_cast<std::uint8_t>(input);
        if (pressed)
            this->_keys |= bit;
        else
            this->_keys &= static_cast<std::uint8_t>(~bit);
        this->_context.network.setKeys(this->_keys);
    });
}

void client::GameScene::addSprite(server::EntityType type, const std::string& path, Vector2 size)
{
    if (!std::filesystem::exists(path)) {
        std::cerr << "[Client] missing sprite " << path << std::endl;
        return;
    }
    this->_sprites.insert_or_assign(type, OrByte::Sprite(path, size));
}

void client::GameScene::applySnapshot(const std::vector<EntityView>& entities)
{
    const auto rate = static_cast<float>(this->_view.snapshotRate);
    std::unordered_set<std::uint32_t> alive;
    for (const EntityView& remote : entities) {
        alive.insert(remote.id);
        const Vector2 position(static_cast<float>(remote.x), static_cast<float>(remote.y));
        const auto local = this->_entities.find(remote.id);
        if (local == this->_entities.end()) {
            this->_entities.emplace(remote.id, this->spawn(static_cast<server::EntityType>(remote.type), position));
            continue;
        }
        const auto *transform = this->_world.getComponent<OrByte::Transform>(local->second);
        auto *velocity = this->_world.getComponent<OrByte::Velocity>(local->second);
        if (transform != nullptr && velocity != nullptr)
            velocity->velocity = Vector2((position.x - transform->position.x) * rate, (position.y - transform->position.y) * rate);
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

int client::GameScene::spawn(server::EntityType type, Vector2 position)
{
    const int entity = this->_world.createEntity();
    this->_world.addComponentToEntity<OrByte::Transform>(entity, position, Vector2(1.0f, 1.0f), 0.0f);
    this->_world.addComponentToEntity<OrByte::Velocity>(entity);
    const auto sprite = this->_sprites.find(type);
    if (sprite != this->_sprites.end())
        this->_world.addComponentToEntity<OrByte::Sprite>(entity, sprite->second);
    return entity;
}
