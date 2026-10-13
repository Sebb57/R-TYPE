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
#include <iostream>
#include <limits>
#include <utility>
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/Systems/HealthSystem.hpp"
#include "GameEngine/Systems/PhysicSystem.hpp"
#include "constants.hpp"

namespace {

Vector2 spawnPosition(server::PlayerSlot slot)
{
    return {OrByte::constants::SERVER_WINDOW_X / 10.0f, static_cast<float>(slot + 1) * OrByte::constants::SERVER_WINDOW_Y / 5.0f};
}

} // namespace

server::GameInstance::GameInstance(RoomId room, unsigned tickRate, unsigned snapshotRate, OutgoingSink sink)
    : _room(room)
    , _tickRate(std::max(tickRate, 1U))
    , _snapshotRate(std::max(snapshotRate, 1U))
    , _world(std::make_unique<World>())
    , _healthSystem(std::make_unique<OrByte::HealthSystem>(*this->_world))
    , _physicSystem(std::make_unique<OrByte::PhysicSystem>(*this->_world))
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
            if (!this->_lastSnapshotCount || *this->_lastSnapshotCount != snapshot.entities.size()) {
                std::cerr << "[GAME] room=" << this->_room << " snapshot tick=" << tick
                    << " entities=" << snapshot.entities.size() << '\n';
                this->_lastSnapshotCount = snapshot.entities.size();
            }
            this->emit(Target(Target::Kind::All), std::move(snapshot));
        }
        std::this_thread::sleep_until(nextTick);
    }
    this->_finished = true;
}

void server::GameInstance::bindInputs()
{
    this->_inputHandlers = {
        {InputKey::Up, this->logInput("up")},
        {InputKey::Down, this->logInput("down")},
        {InputKey::Left, this->logInput("left")},
        {InputKey::Right, this->logInput("right")},
        {InputKey::Fire, this->logInput("fire")},
    };
}

server::GameInstance::InputHandler server::GameInstance::logInput(std::string action) const
{
    return [this, action = std::move(action)](PlayerSlot slot, int, bool pressed) {
        std::cerr << "[GAME] room=" << this->_room << " slot=" << static_cast<unsigned>(slot) << ' ' << action << (pressed ? " pressed" : " released") << '\n';
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
    const int entity = this->_world->createEntity();
    this->_world->addComponentToEntity<OrByte::Transform>(entity, spawnPosition(event.slot), Vector2(1.0f, 1.0f), 0.0f);
    this->_playerEntities.emplace(event.slot, entity);
    std::cerr << "[GAME] room=" << this->_room << " player=" << event.name << " slot=" << static_cast<unsigned>(event.slot) << " entity=" << static_cast<std::uint32_t>(entity + 1) << " entities=" << this->_playerEntities.size() << '\n';
}

void server::GameInstance::handle(const PlayerLeft& event)
{
    const auto player = this->_playerEntities.find(event.slot);
    if (player == this->_playerEntities.end())
        return;
    this->_world->destroyEntity(player->second);
    this->_playerEntities.erase(player);
    this->_playerInputs.erase(event.slot);
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
    this->_physicSystem->update();
    this->_healthSystem->update();
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
        const auto owner = owners.find(entity);
        const auto type = owner != owners.end() ? EntityType::Player : EntityType::Unknown;
        const std::uint8_t flags = owner != owners.end() ? static_cast<std::uint8_t>(owner->second & EntitySlotMask) : 0;
        snapshot.entities.push_back(EntityDto(static_cast<std::uint32_t>(entity + 1), static_cast<std::uint8_t>(type),
            toCoordinate(transform->position.x), toCoordinate(transform->position.y), flags));
    }
    return snapshot;
}

void server::GameInstance::emit(Target target, std::variant<SnapshotDto, GameEventDto> payload)
{
    if (this->_sink)
        this->_sink(OutboundMessage(this->_room, target, std::move(payload)));
}
