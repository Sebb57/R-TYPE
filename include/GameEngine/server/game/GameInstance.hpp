/**
 * @file GameInstance.hpp
 * @brief solo game simulation, rendered by its player and streamed to spectators
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_GAME_GAMEINSTANCE_HPP
#define GAMEENGINE_SERVER_GAME_GAMEINSTANCE_HPP

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "GameEngine/Event.hpp"
#include "GameEngine/server/common/ServerMessages.hpp"
#include "GameEngine/server/game/BoundedQueue.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"

class World;
namespace OrByte {
class IAudio;
class IGraphic;
}

namespace server {

class GameInstance {
    public:
        using Setup = std::function<void(World&)>;

        GameInstance(OrByte::IGraphic& graphic, unsigned tickRate, unsigned snapshotRate, const Setup& setup);
        ~GameInstance();

        void start();
        void stop();
        bool push(const OrByte::Event& event);
        void render();
        void setSink(SnapshotSink sink);
        [[nodiscard]] bool finished() const;

    private:
        struct LoadedSystem {
            std::string name;
            std::unique_ptr<OrByte::DynamicLibrary> loader;
            std::unique_ptr<OrByte::ISystem> system;
        };

        void loadSystems(OrByte::IGraphic& graphic);
        [[nodiscard]] OrByte::ISystem *createSystem(const std::string& name, OrByte::DynamicLibrary& loader, OrByte::IGraphic& graphic);
        [[nodiscard]] bool loadAudio();
        void run(std::stop_token);
        void applyInbound();
        void update();
        [[nodiscard]] SnapshotDto makeSnapshot();

        unsigned _tickRate;
        unsigned _snapshotRate;
        std::unique_ptr<OrByte::DynamicLibrary> _audioLoader;
        std::unique_ptr<OrByte::IAudio> _audio;
        std::unique_ptr<World> _world;
        std::vector<LoadedSystem> _systems;
        LoadedSystem _display;
        BoundedQueue<OrByte::Event> _inbox{256};
        std::mutex _mutex;
        SnapshotSink _sink;
        std::uint32_t _tick = 0;
        std::atomic<bool> _finished{false};
        std::jthread _thread;
};

} // namespace server

#endif // GAMEENGINE_SERVER_GAME_GAMEINSTANCE_HPP
