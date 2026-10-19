/**
 * @file GameInstance.hpp
 * @brief room game simulation interface
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef SERVER_GAME_GAMEINSTANCE_HPP
#define SERVER_GAME_GAMEINSTANCE_HPP

#include <atomic>
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <thread>
#include "BoundedQueue.hpp"
#include "Chronometer.hpp"
#include "GameEngine/Systems/ColliderSystem.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include "protocol/UdpProtocol.hpp"
#include "server/common/ServerMessages.hpp"

class World;
namespace OrByte {
class HealthSystem;
class PhysicSystem;
}
namespace RType {
class Playeur;
class Bonus;
}

namespace server {

class GameInstance {
    public:
        GameInstance(RoomId room, unsigned tickRate, unsigned snapshotRate, OutgoingSink sink);
        ~GameInstance();

        void start();
        void stop();
        bool push(InboundEvent event);
        [[nodiscard]] bool finished() const;

    private:
        using InputHandler = std::function<void(PlayerSlot slot, int entity, bool pressed)>;
        using HoldHandler = std::function<void(PlayerSlot slot, int entity)>;

        void run(std::stop_token);
        void bindInputs();
        void applyInbound();
        void handle(const PlayerJoined& event);
        void handle(const PlayerLeft& event);
        void handle(const PlayerInput& event);
        void update();
        void holdInputs();
        void steer(PlayerSlot slot, int entity);
        void fire(PlayerSlot slot, int entity);
        void keepPlayersInBounds();
        void removeOutOfBounds();
        int spawn(EntityType type, Vector2 position, PlayerSlot playerSlot = 0, bool extraShotBonus = false);
        void destroyEnemy(int entity);
        void despawn(int entity);
        [[nodiscard]] bool isHeld(PlayerSlot slot, InputKey key) const;
        [[nodiscard]] SnapshotDto makeSnapshot(std::uint32_t tick);
        void emit(Target, std::variant<SnapshotDto, GameEventDto>);
        void emitCollisionEffect(CollisionEffect, int entity, Vector2 position);

        RoomId _room;
        unsigned _tickRate;
        unsigned _snapshotRate;
        std::unique_ptr<World> _world;
        std::unique_ptr<OrByte::HealthSystem> _healthSystem;
        std::unique_ptr<OrByte::PhysicSystem> _physicSystem;
        std::unique_ptr<OrByte::ColliderSystem> _colliderSystem;
        BoundedQueue<InboundEvent> _inbox{256};
        OutgoingSink _sink;
        std::map<InputKey, InputHandler> _inputHandlers;
        std::map<InputKey, HoldHandler> _holdHandlers;
        std::unordered_map<PlayerSlot, std::uint8_t> _playerInputs;
        std::unordered_map<PlayerSlot, int> _playerEntities;
        std::unordered_map<PlayerSlot, OrByte::Chronometer> _fireClocks;
        std::unordered_map<int, EntityType> _types;
        std::unordered_map<int, std::unique_ptr<RType::Playeur>> _playeurs;
        std::unordered_map<int, std::unique_ptr<RType::Bonus>> _bonuses;
        std::unordered_map<int, PlayerSlot> _projectileOwners;
        std::array<std::uint32_t, 4> _scores{};
        std::uint32_t _nextEventId = 1;
        std::optional<std::size_t> _lastPlayerCount;
        OrByte::Chronometer _enemyClock;
        double _nextEnemySpawn = 2.0;
        OrByte::Chronometer _bonusClock;
        double _nextBonusSpawn = 10.0;
        OrByte::Chronometer _gameClock;
        bool _gameOver = false;
        std::atomic<bool> _finished{false};
        std::jthread _thread;
};

} // namespace server

#endif // SERVER_GAME_GAMEINSTANCE_HPP
