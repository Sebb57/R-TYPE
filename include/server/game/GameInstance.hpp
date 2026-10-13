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
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <thread>
#include "BoundedQueue.hpp"
#include "protocol/UdpProtocol.hpp"
#include "server/common/ServerMessages.hpp"

class World;
namespace OrByte {
class HealthSystem;
class PhysicSystem;
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

        void run(std::stop_token);
        void bindInputs();
        [[nodiscard]] InputHandler logInput(std::string action) const;
        void applyInbound();
        void handle(const PlayerJoined& event);
        void handle(const PlayerLeft& event);
        void handle(const PlayerInput& event);
        void update();
        [[nodiscard]] SnapshotDto makeSnapshot(std::uint32_t tick);
        void emit(Target, std::variant<SnapshotDto, GameEventDto>);

        RoomId _room;
        unsigned _tickRate;
        unsigned _snapshotRate;
        std::unique_ptr<World> _world;
        std::unique_ptr<OrByte::HealthSystem> _healthSystem;
        std::unique_ptr<OrByte::PhysicSystem> _physicSystem;
        BoundedQueue<InboundEvent> _inbox{256};
        OutgoingSink _sink;
        std::map<InputKey, InputHandler> _inputHandlers;
        std::unordered_map<PlayerSlot, std::uint8_t> _playerInputs;
        std::unordered_map<PlayerSlot, int> _playerEntities;
        std::optional<std::size_t> _lastSnapshotCount;
        std::atomic<bool> _finished{false};
        std::jthread _thread;
};

} // namespace server

#endif // SERVER_GAME_GAMEINSTANCE_HPP
