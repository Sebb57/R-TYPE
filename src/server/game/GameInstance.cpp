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
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>
#include "GameEngine/ECS/Components/Orbit.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/Systems/HealthSystem.hpp"
#include "GameEngine/Systems/PhysicSystem.hpp"
#include "protocol/UdpProtocol.hpp"

namespace {

// each slot spawns on its own side of the center (right, bottom, left, top) so circles never overlap
float spawnAngle(server::PlayerSlot slot)
{
    return static_cast<float>(slot) * std::numbers::pi_v<float> / 2.0f;
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
{}

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

void server::GameInstance::applyInbound()
{
    for (InboundEvent& event : this->_inbox.drain()) {
        if (const auto* joined = std::get_if<PlayerJoined>(&event)) {
            if (this->_playerEntities.contains(joined->slot))
                continue;
            const float radius = 60.0f + static_cast<float>(joined->slot) * 25.0f;
            const float angle = spawnAngle(joined->slot);
            const Vector2 center(500.0f, 500.0f);
            int entity = this->_world->createEntity();
            this->_world->addComponentToEntity<Transform>(entity,
                Vector2(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius), Vector2(1.0f, 1.0f), 0.0f);
            this->_playerEntities.emplace(joined->slot, entity);
            std::cerr << "[GAME] room=" << this->_room << " player=" << joined->name
                << " slot=" << static_cast<unsigned>(joined->slot) << " entity="
                << static_cast<std::uint32_t>(entity + 1) << " entities=" << this->_playerEntities.size() << '\n';
            continue;
        }
        if (const auto* left = std::get_if<PlayerLeft>(&event)) {
            auto player = this->_playerEntities.find(left->slot);
            if (player != this->_playerEntities.end()) {
                this->_world->destroyEntity(player->second);
                this->_playerEntities.erase(player);
                this->_playerInputs.erase(left->slot);
            }
            continue;
        }
        if (const auto* input = std::get_if<PlayerInput>(&event)) {
            const std::uint8_t previousKeys = this->_playerInputs[input->slot];
            this->_playerInputs[input->slot] = input->keys;
            const auto player = this->_playerEntities.find(input->slot);
            if ((previousKeys & 0x10) != 0 || (input->keys & 0x10) == 0 ||
                player == this->_playerEntities.end() ||
                this->_world->getComponent<Orbit>(player->second) != nullptr)
                continue;
            const float radius = 60.0f + static_cast<float>(input->slot) * 25.0f;
            this->_world->addComponentToEntity<Orbit>(player->second, Vector2(500.0f, 500.0f), radius,
                0.75f + static_cast<float>(input->slot) * 0.2f);
            this->_world->getComponent<Orbit>(player->second)->angle = spawnAngle(input->slot);
            std::cerr << "[GAME] room=" << this->_room << " slot=" << static_cast<unsigned>(input->slot)
                << " space pressed; orbit enabled\n";
        }
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
        auto transform = this->_world->getComponent<Transform>(entity);
        if (transform == nullptr)
            continue;
        const auto toCoordinate = [](float value) {
            return static_cast<std::int16_t>(std::clamp(value,
                static_cast<float>(std::numeric_limits<std::int16_t>::min()),
                static_cast<float>(std::numeric_limits<std::int16_t>::max())));
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
