/**
 * @file GameInstance.cpp
 * @brief room game simulation implementation
 *
 * @author Seb
 * @date 2026-10-12
**/

#include "server/game/GameInstance.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/World.hpp"
#include "constants.hpp"

namespace {

Vector2 spawnPosition(server::PlayerSlot slot)
{
    return {OrByte::constants::SERVER_WINDOW_X / 10.0f, static_cast<float>(slot + 1) * OrByte::constants::SERVER_WINDOW_Y / 5.0f};
}

bool outsideScreen(Vector2 position, Vector2 size)
{
    return position.x + size.x / 2.0f < 0.0f || position.x - size.x / 2.0f > OrByte::constants::SERVER_WINDOW_X
        || position.y + size.y / 2.0f < 0.0f || position.y - size.y / 2.0f > OrByte::constants::SERVER_WINDOW_Y;
}

} // namespace

server::GameInstance::GameInstance(RoomId room, unsigned tickRate, unsigned snapshotRate, OutgoingSink sink)
    : _room(room)
    , _tickRate(std::max(tickRate, 1U))
    , _snapshotRate(std::max(snapshotRate, 1U))
    , _world(std::make_unique<World>())
    , _healthLoader("./plugins/systems/HealthSystem.so")
    , _physicLoader("./plugins/systems/PhysicSystem.so")
    , _healthSystem(this->_healthLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
    , _physicSystem(this->_physicLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
    , _sink(std::move(sink))
{
    this->bindInputs();
}

server::GameInstance::~GameInstance()
{
    this->stop();
}

void server::GameInstance::start()
{
    if (!this->_thread.joinable())
        this->_thread = std::jthread([this](std::stop_token token) { this->run(token); });
}

void server::GameInstance::stop()
{
    if (this->_thread.joinable()) {
        this->_thread.request_stop();
        this->_thread.join();
    }
    this->_finished = true;
}

bool server::GameInstance::push(InboundEvent event)
{
    return this->_inbox.push(std::move(event));
}

bool server::GameInstance::finished() const
{
    return this->_finished.load();
}

void server::GameInstance::run(std::stop_token token)
{
    const auto tickDuration = std::chrono::duration<double>(1.0 / this->_tickRate);
    const unsigned snapshotInterval = std::max(this->_tickRate / this->_snapshotRate, 1U);
    auto nextTick = server::Clock::now();
    std::uint32_t tick = 0;

    while (!token.stop_requested()) {
        nextTick += std::chrono::duration_cast<server::Clock::duration>(tickDuration);
        this->applyInbound();
        this->update();
        ++tick;
        if (tick % snapshotInterval == 0) {
            SnapshotDto snapshot = this->makeSnapshot(tick);
            if (!this->_lastPlayerCount || *this->_lastPlayerCount != this->_playerEntities.size()) {
                std::cerr << "[GAME] room=" << this->_room << " snapshot tick=" << tick
                    << " players=" << this->_playerEntities.size() << " entities=" << snapshot.entities.size() << '\n';
                this->_lastPlayerCount = this->_playerEntities.size();
            }
            this->emit(Target(Target::Kind::All), std::move(snapshot));
        }
        std::this_thread::sleep_until(nextTick);
    }
    this->_finished = true;
}

void server::GameInstance::bindInputs()
{
    const InputHandler steer = [this](PlayerSlot slot, int entity, bool) { this->steer(slot, entity); };
    this->_inputHandlers = {
        {InputKey::Up, steer},
        {InputKey::Down, steer},
        {InputKey::Left, steer},
        {InputKey::Right, steer},
    };
    this->_holdHandlers = {
        {InputKey::Fire, [this](PlayerSlot slot, int entity) { this->fire(slot, entity); }},
    };
}

void server::GameInstance::applyInbound()
{
    for (const InboundEvent& event : this->_inbox.drain())
        std::visit([this](const auto& message) { this->handle(message); }, event);
}

void server::GameInstance::handle(const PlayerJoined& event)
{
    if (this->_playerEntities.contains(event.slot))
        return;
    const int entity = this->spawn(EntityType::Player, spawnPosition(event.slot),
        Vector2(OrByte::constants::PLAYER_WIDTH, OrByte::constants::PLAYER_HEIGHT), Vector2(0.0f, 0.0f));
    this->_playerEntities.emplace(event.slot, entity);
    this->_fireClocks.emplace(event.slot, OrByte::Chronometer());
    std::cerr << "[GAME] room=" << this->_room << " player=" << event.name << " slot=" << static_cast<unsigned>(event.slot) << " entity=" << static_cast<std::uint32_t>(entity + 1) << " entities=" << this->_playerEntities.size() << '\n';
}

void server::GameInstance::handle(const PlayerLeft& event)
{
    const auto player = this->_playerEntities.find(event.slot);
    if (player == this->_playerEntities.end())
        return;
    this->despawn(player->second);
    this->_playerEntities.erase(player);
    this->_playerInputs.erase(event.slot);
    this->_fireClocks.erase(event.slot);
}

void server::GameInstance::handle(const PlayerInput& event)
{
    const auto player = this->_playerEntities.find(event.slot);
    if (player == this->_playerEntities.end())
        return;
    const auto changed = static_cast<std::uint8_t>(this->_playerInputs[event.slot] ^ event.keys);
    this->_playerInputs[event.slot] = event.keys;
    for (const auto& [key, handler] : this->_inputHandlers) {
        const auto bit = static_cast<std::uint8_t>(key);
        if ((changed & bit) != 0)
            handler(event.slot, player->second, (event.keys & bit) != 0);
    }
}

void server::GameInstance::update()
{
    this->holdInputs();
    this->_physicSystem->update();
    this->_healthSystem->update();
    this->keepPlayersInBounds();
    this->removeOutOfBounds();
}

void server::GameInstance::holdInputs()
{
    for (const auto& [slot, entity] : this->_playerEntities)
        for (const auto& [key, handler] : this->_holdHandlers)
            if (this->isHeld(slot, key))
                handler(slot, entity);
}

void server::GameInstance::steer(PlayerSlot slot, int entity)
{
    auto *velocity = this->_world->getComponent<OrByte::Velocity>(entity);
    if (velocity == nullptr)
        return;
    const auto axis = [this, slot](InputKey positive, InputKey negative) {
        return static_cast<float>(this->isHeld(slot, positive)) - static_cast<float>(this->isHeld(slot, negative));
    };
    const Vector2 direction(axis(InputKey::Right, InputKey::Left), axis(InputKey::Down, InputKey::Up));
    const float length = std::hypot(direction.x, direction.y);
    const float speed = length > 0.0f ? OrByte::constants::PLAYER_SPEED / length : 0.0f;
    velocity->velocity = Vector2(direction.x * speed, direction.y * speed);
}

void server::GameInstance::fire(PlayerSlot slot, int entity)
{
    OrByte::Chronometer& clock = this->_fireClocks[slot];
    const auto *transform = this->_world->getComponent<OrByte::Transform>(entity);
    if (transform == nullptr || clock.getElapsedTime() < 1.0 / OrByte::constants::FIRE_RATE)
        return;
    clock.reset();
    const Vector2 muzzle(transform->position.x + (OrByte::constants::PLAYER_WIDTH + OrByte::constants::PROJECTILE_WIDTH) / 2.0f, transform->position.y);
    this->spawn(EntityType::Projectile, muzzle, Vector2(OrByte::constants::PROJECTILE_WIDTH, OrByte::constants::PROJECTILE_HEIGHT),
        Vector2(OrByte::constants::PROJECTILE_SPEED, 0.0f));
}

void server::GameInstance::keepPlayersInBounds()
{
    for (const auto& [slot, entity] : this->_playerEntities) {
        auto *transform = this->_world->getComponent<OrByte::Transform>(entity);
        const auto *collider = this->_world->getComponent<Collider>(entity);
        if (transform == nullptr || collider == nullptr)
            continue;
        transform->position.x = std::clamp(transform->position.x, collider->size.x / 2.0f, OrByte::constants::SERVER_WINDOW_X - collider->size.x / 2.0f);
        transform->position.y = std::clamp(transform->position.y, collider->size.y / 2.0f, OrByte::constants::SERVER_WINDOW_Y - collider->size.y / 2.0f);
    }
}

void server::GameInstance::removeOutOfBounds()
{
    std::vector<int> outside;
    for (const auto& [entity, type] : this->_types) {
        const auto *transform = this->_world->getComponent<OrByte::Transform>(entity);
        const auto *collider = this->_world->getComponent<Collider>(entity);
        if (type != EntityType::Player && transform != nullptr && collider != nullptr && outsideScreen(transform->position, collider->size))
            outside.push_back(entity);
    }
    for (int entity : outside)
        this->despawn(entity);
}

int server::GameInstance::spawn(EntityType type, Vector2 position, Vector2 size, Vector2 velocity)
{
    const int entity = this->_world->createEntity();
    this->_world->addComponentToEntity<OrByte::Transform>(entity, position, Vector2(1.0f, 1.0f), 0.0f);
    this->_world->addComponentToEntity<Collider>(entity, size);
    this->_world->addComponentToEntity<OrByte::Velocity>(entity, velocity);
    this->_types.emplace(entity, type);
    return entity;
}

void server::GameInstance::despawn(int entity)
{
    this->_world->destroyEntity(entity);
    this->_types.erase(entity);
}

bool server::GameInstance::isHeld(PlayerSlot slot, InputKey key) const
{
    const auto keys = this->_playerInputs.find(slot);
    return keys != this->_playerInputs.end() && (keys->second & static_cast<std::uint8_t>(key)) != 0;
}

server::SnapshotDto server::GameInstance::makeSnapshot(std::uint32_t tick)
{
    SnapshotDto snapshot;
    snapshot.tick = tick;
    std::unordered_map<int, PlayerSlot> owners;
    for (const auto& [slot, entity] : this->_playerEntities)
        owners.emplace(entity, slot);
    for (int entity : this->_world->getExistingEntities()) {
        auto transform = this->_world->getComponent<OrByte::Transform>(entity);
        if (transform == nullptr)
            continue;
        const auto toCoordinate = [](float value) {
            return static_cast<std::int16_t>(std::clamp(value, static_cast<float>(std::numeric_limits<std::int16_t>::min()), static_cast<float>(std::numeric_limits<std::int16_t>::max())));
        };
        const auto type = this->_types.find(entity);
        const auto owner = owners.find(entity);
        const std::uint8_t flags = owner != owners.end() ? static_cast<std::uint8_t>(owner->second & EntitySlotMask) : 0;
        snapshot.entities.push_back(EntityDto(static_cast<std::uint32_t>(entity + 1),
            static_cast<std::uint8_t>(type != this->_types.end() ? type->second : EntityType::Unknown),
            toCoordinate(transform->position.x), toCoordinate(transform->position.y), flags));
    }
    return snapshot;
}

void server::GameInstance::emit(Target target, std::variant<SnapshotDto, GameEventDto> payload)
{
    if (this->_sink)
        this->_sink(OutboundMessage(this->_room, target, std::move(payload)));
}
