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
#include <memory>
#include <unordered_map>
#include <thread>
#include "BoundedQueue.hpp"
#include "server/common/ServerMessages.hpp"

class World;

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
        void run(std::stop_token);
        void applyInbound();
        void update(float dtSeconds);
        [[nodiscard]] SnapshotDto makeSnapshot(std::uint32_t tick);
        void emit(Target, std::variant<SnapshotDto, GameEventDto>);

        RoomId _room;
        unsigned _tickRate;
        unsigned _snapshotRate;
        std::unique_ptr<World> _world;
        BoundedQueue<InboundEvent> _inbox{256};
        OutgoingSink _sink;
        // Latest validated input state per player, consumed by the future ECS input system.
        std::unordered_map<PlayerSlot, std::uint8_t> _playerInputs;
        std::atomic<bool> _finished{false};
        std::jthread _thread;
};

} // namespace server

#endif // SERVER_GAME_GAMEINSTANCE_HPP
