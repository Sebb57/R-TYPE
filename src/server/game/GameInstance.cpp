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
#include <random>
#include <utility>
#include <vector>
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "Orbyte/Bonus.hpp"
#include "Orbyte/Ennemies.hpp"
#include "Orbyte/Playeur.hpp"
#include "Orbyte/Projectiles.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/Systems/ColliderSystem.hpp"
#include "GameEngine/Systems/HealthSystem.hpp"
#include "GameEngine/Systems/PhysicSystem.hpp"
#include "constants.hpp"

namespace {

constexpr double GameDurationSeconds = 180.0;

Vector2 spawnPosition(server::PlayerSlot slot)
{
    return {OrByte::constants::SERVER_WINDOW_X / 10.0f, static_cast<float>(slot + 1) * OrByte::constants::SERVER_WINDOW_Y / 5.0f};
}

double nextEnemySpawnDelay()
{
    thread_local std::mt19937 engine(std::random_device{}());
    return std::uniform_real_distribution<double>(0.5, 1.5)(engine);
}

double nextBonusSpawnDelay()
{
    thread_local std::mt19937 engine(std::random_device{}());
    return std::uniform_real_distribution<double>(8.0, 15.0)(engine);
}

} // namespace

server::GameInstance::GameInstance(RoomId room, unsigned tickRate, unsigned snapshotRate, OutgoingSink sink)
    : _room(room)
    , _tickRate(std::max(tickRate, 1U))
    , _snapshotRate(std::max(snapshotRate, 1U))
    , _world(std::make_unique<World>())
    , _healthLoader(std::filesystem::path(OrByte::constants::SYSTEMS_DIR) / OrByte::constants::pluginFile("HealthSystem"))
    , _physicLoader(std::filesystem::path(OrByte::constants::SYSTEMS_DIR) / OrByte::constants::pluginFile("PhysicSystem"))
    , _colliderLoader(std::filesystem::path(OrByte::constants::SYSTEMS_DIR) / OrByte::constants::pluginFile("ColliderSystem"))
    , _healthSystem(this->_healthLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
    , _physicSystem(this->_physicLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
    , _colliderSystem(this->_colliderLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
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
    if (!this->_thread.joinable()) {
        this->_gameClock.reset();
        this->_gameOver = false;
        this->_thread = std::jthread([this](std::stop_token token) { this->run(token); });
    }
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
        if (tick % snapshotInterval == 0 || this->_gameOver) {
            SnapshotDto snapshot = this->makeSnapshot(tick);
            if (!this->_lastPlayerCount || *this->_lastPlayerCount != this->_playerEntities.size()) {
                std::cerr << "[GAME] room=" << this->_room << " snapshot tick=" << tick
                    << " players=" << this->_playerEntities.size() << " entities=" << snapshot.entities.size() << '\n';
                this->_lastPlayerCount = this->_playerEntities.size();
            }
            this->emit(Target(Target::Kind::All), std::move(snapshot));
        }
        if (this->_gameOver) {
            this->_finished = true;
            break;
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
    const int entity = this->spawn(EntityType::Player, spawnPosition(event.slot), event.slot);
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
    if (this->_gameClock.getElapsedTime() >= GameDurationSeconds) {
        this->_gameOver = true;
        return;
    }
    if (this->_enemyClock.getElapsedTime() >= this->_nextEnemySpawn) {
        this->_enemyClock.reset();
        this->_nextEnemySpawn = nextEnemySpawnDelay();
        this->spawn(EntityType::Enemy, {});
    }
    if (this->_bonusClock.getElapsedTime() >= this->_nextBonusSpawn) {
        this->_bonusClock.reset();
        this->_nextBonusSpawn = nextBonusSpawnDelay();
        this->spawn(EntityType::Bonus, {}, 0, true);
    }
    this->holdInputs();
    this->_physicSystem->update();
    this->_colliderSystem->update();
    for (const auto& [first, second] : this->_colliderSystem->getEntityPairs()) {
        const EntityType firstType = this->_types.contains(first) ? this->_types.at(first) : EntityType::Unknown;
        const EntityType secondType = this->_types.contains(second) ? this->_types.at(second) : EntityType::Unknown;
        if (firstType == EntityType::Bonus || secondType == EntityType::Bonus) {
            const int bonus = firstType == EntityType::Bonus ? first : second;
            const int player = firstType == EntityType::Player ? first : second;
            const auto bonusEntity = this->_bonuses.find(bonus);
            const auto playerEntity = this->_playeurs.find(player);
            if ((firstType == EntityType::Player || secondType == EntityType::Player)
                && bonusEntity != this->_bonuses.end() && playerEntity != this->_playeurs.end()) {
                if (const auto *playerTransform = this->_world->getComponent<OrByte::Transform>(player))
                    this->emitCollisionEffect(CollisionEffect::BluePlayer, player, playerTransform->position);
                bonusEntity->second->ApplyTo(this->_world.get(), *playerEntity->second);
                this->despawn(bonus);
            }
            continue;
        }
        const auto *ownerA = this->_world->getComponent<OrByte::Owner>(first);
        const auto *ownerB = this->_world->getComponent<OrByte::Owner>(second);
        if (ownerA && ownerB && ownerA->team != ownerB->team) {
            if (firstType == EntityType::Projectile || secondType == EntityType::Projectile) {
                OrByte::Projectiles::CheckCollision(this->_world.get(), Vector2(static_cast<float>(first), static_cast<float>(second)));
                const int enemy = firstType == EntityType::Enemy ? first : (secondType == EntityType::Enemy ? second : -1);
                if (enemy >= 0) {
                    if (const auto *transform = this->_world->getComponent<OrByte::Transform>(enemy))
                        this->emitCollisionEffect(CollisionEffect::RedEnemy, enemy, transform->position);
                    const auto *health = this->_world->getComponent<OrByte::Health>(enemy);
                    if (health && health->currentHealth <= 0) {
                        const int projectile = firstType == EntityType::Projectile ? first : second;
                        const auto shooter = this->_projectileOwners.find(projectile);
                        if (shooter != this->_projectileOwners.end() && shooter->second < this->_scores.size())
                            this->_scores[shooter->second] += 100;
                        this->destroyEnemy(enemy);
                    }
                }
            } else if ((firstType == EntityType::Player && secondType == EntityType::Enemy)
                || (firstType == EntityType::Enemy && secondType == EntityType::Player)) {
                const int player = firstType == EntityType::Player ? first : second;
                const int enemy = firstType == EntityType::Enemy ? first : second;
                const auto *playerTransform = this->_world->getComponent<OrByte::Transform>(player);
                const auto *enemyTransform = this->_world->getComponent<OrByte::Transform>(enemy);
                if (playerTransform)
                    this->emitCollisionEffect(CollisionEffect::RedPlayer, player, playerTransform->position);
                if (enemyTransform)
                    this->emitCollisionEffect(CollisionEffect::RedEnemy, enemy, enemyTransform->position);
                const auto playerEntity = this->_playeurs.find(player);
                if (playerEntity != this->_playeurs.end())
                    playerEntity->second->ReceiveEnemyHit(this->_world.get(), 25);
                for (const auto& [slot, playerId] : this->_playerEntities)
                    if (playerId == player && slot < this->_scores.size())
                        this->_scores[slot] += 100;
                this->destroyEnemy(enemy);
            }
            if (firstType == EntityType::Projectile)
                this->despawn(first);
            if (secondType == EntityType::Projectile)
                this->despawn(second);
        }
    }
    std::unordered_map<int, Vector2> playerPositions;
    for (const auto& [slot, player] : this->_playerEntities) {
        (void)slot;
        if (const auto *transform = this->_world->getComponent<OrByte::Transform>(player))
            playerPositions.emplace(player, transform->position);
    }
    this->_healthSystem->update();
    for (auto entity = this->_types.begin(); entity != this->_types.end();) {
        if (this->_world->entityExists(entity->first)) {
            ++entity;
            continue;
        }
        for (auto player = this->_playerEntities.begin(); player != this->_playerEntities.end();) {
            if (player->second == entity->first) {
                const auto position = playerPositions.find(entity->first);
                this->emitCollisionEffect(CollisionEffect::PlayerDestroyed, entity->first,
                    position != playerPositions.end() ? position->second : Vector2{});
                this->_playerInputs.erase(player->first);
                this->_fireClocks.erase(player->first);
                player = this->_playerEntities.erase(player);
            } else {
                ++player;
            }
        }
        this->_playeurs.erase(entity->first);
        this->_bonuses.erase(entity->first);
        entity = this->_types.erase(entity);
    }
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
    const auto player = this->_playeurs.find(entity);
    if (player == this->_playeurs.end())
        return;
    const auto axis = [this, slot](InputKey positive, InputKey negative) {
        return static_cast<float>(this->isHeld(slot, positive)) - static_cast<float>(this->isHeld(slot, negative));
    };
    const Vector2 direction(axis(InputKey::Right, InputKey::Left), axis(InputKey::Down, InputKey::Up));
    player->second->Steer(this->_world.get(), direction);
}

void server::GameInstance::fire(PlayerSlot slot, int entity)
{
    OrByte::Chronometer& clock = this->_fireClocks[slot];
    if (this->_world->getComponent<OrByte::Transform>(entity) == nullptr || clock.getElapsedTime() < 1.0 / OrByte::constants::FIRE_RATE)
        return;
    clock.reset();
    const auto player = this->_playeurs.find(entity);
    if (player == this->_playeurs.end())
        return;
    for (int projectile : player->second->Fire(this->_world.get()))
    {
        this->_types.emplace(projectile, EntityType::Projectile);
        this->_projectileOwners.emplace(projectile, slot);
    }
}

void server::GameInstance::keepPlayersInBounds()
{
    for (const auto& [slot, entity] : this->_playerEntities) {
        const auto player = this->_playeurs.find(entity);
        if (player != this->_playeurs.end())
            player->second->KeepInBounds(this->_world.get(), OrByte::constants::SERVER_WINDOW_X, OrByte::constants::SERVER_WINDOW_Y);
    }
}

void server::GameInstance::removeOutOfBounds()
{
    std::vector<int> outside;
    for (const auto& [entity, type] : this->_types) {
        const auto *transform = this->_world->getComponent<OrByte::Transform>(entity);
        const auto *collider = this->_world->getComponent<OrByte::Collider>(entity);
        if (type == EntityType::Player || transform == nullptr || collider == nullptr)
            continue;
        const float halfWidth = collider->size.x / 2.0f;
        const float halfHeight = collider->size.y / 2.0f;
        const bool leftOfScreen = transform->position.x + halfWidth < 0.0f;
        const bool rightOfScreen = transform->position.x - halfWidth > OrByte::constants::SERVER_WINDOW_X;
        const bool aboveScreen = transform->position.y + halfHeight < 0.0f;
        const bool belowScreen = transform->position.y - halfHeight > OrByte::constants::SERVER_WINDOW_Y;
        const bool movesLeft = type == EntityType::Enemy || type == EntityType::Bonus;
        if ((movesLeft && leftOfScreen) || (!movesLeft && rightOfScreen) || aboveScreen || belowScreen)
            outside.push_back(entity);
    }
    for (int entity : outside)
        this->despawn(entity);
}

int server::GameInstance::spawn(EntityType type, Vector2 position, PlayerSlot playerSlot, bool extraShotBonus)
{
    int entity = -1;
    if (type == EntityType::Player) {
        auto player = std::make_unique<OrByte::Playeur>(this->_world.get());
        player->CreateServeurComponent(this->_world.get(), position, playerSlot);
        entity = player->GetID();
        this->_playeurs.emplace(entity, std::move(player));
    } else if (type == EntityType::Enemy) {
        OrByte::Ennemies enemy(this->_world.get());
        enemy.CreateServeurComponent(this->_world.get());
        entity = enemy.GetID();
    } else if (type == EntityType::Bonus) {
        auto bonus = std::make_unique<OrByte::Bonus>(this->_world.get(), extraShotBonus);
        bonus->CreateServeurBonus(this->_world.get(), position);
        entity = bonus->GetID();
        this->_bonuses.emplace(entity, std::move(bonus));
    } else {
        entity = this->_world->createEntity();
    }
    this->_types.emplace(entity, type);
    return entity;
}

void server::GameInstance::destroyEnemy(int entity)
{
    const auto *transform = this->_world->getComponent<OrByte::Transform>(entity);
    const Vector2 position = transform ? transform->position : Vector2{};
    this->emitCollisionEffect(CollisionEffect::EnemyDestroyed, entity, position);
    if (OrByte::Ennemies::ShouldDropBonus())
        this->spawn(EntityType::Bonus, position);
    this->despawn(entity);
}

void server::GameInstance::despawn(int entity)
{
    this->_world->destroyEntity(entity);
    this->_types.erase(entity);
    this->_playeurs.erase(entity);
    this->_bonuses.erase(entity);
    this->_projectileOwners.erase(entity);
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
    const double elapsed = this->_gameClock.getElapsedTime();
    snapshot.remainingSeconds = elapsed >= GameDurationSeconds ? 0 : static_cast<std::uint16_t>(std::ceil(GameDurationSeconds - elapsed));
    snapshot.gameOver = this->_gameOver;
    snapshot.scores = this->_scores;
    for (const auto& [slot, entity] : this->_playerEntities)
        if (slot < 4 && this->_world->entityExists(entity))
            snapshot.aliveMask |= static_cast<std::uint8_t>(1U << slot);
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
        std::uint8_t flags = owner != owners.end() ? static_cast<std::uint8_t>(owner->second & EntitySlotMask) : 0;
        const auto bonus = this->_bonuses.find(entity);
        if (bonus != this->_bonuses.end() && bonus->second->GivesExtraShot())
            flags |= BonusExtraShotFlag;
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

void server::GameInstance::emitCollisionEffect(CollisionEffect type, int entity, Vector2 position)
{
    const auto toCoordinate = [](float value) {
        return static_cast<std::int16_t>(std::clamp(value,
            static_cast<float>(std::numeric_limits<std::int16_t>::min()),
            static_cast<float>(std::numeric_limits<std::int16_t>::max())));
    };
    const auto x = static_cast<std::uint16_t>(toCoordinate(position.x));
    const auto y = static_cast<std::uint16_t>(toCoordinate(position.y));
    const std::uint32_t packedPosition = static_cast<std::uint32_t>(x)
        | (static_cast<std::uint32_t>(y) << 16);
    this->emit(Target(Target::Kind::All), GameEventDto{
        this->_nextEventId++, static_cast<std::uint8_t>(type), static_cast<std::uint32_t>(entity + 1), packedPosition
    });
}
