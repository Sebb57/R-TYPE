/**
 * @file GameScene.hpp
 * @brief in game scene, mirrors server snapshots into the ecs world
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_GAMESCENE_HPP
#define CLIENT_SCENES_GAMESCENE_HPP

#include <cstdint>
#include <array>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "client/network/ClientView.hpp"
#include "client/scenes/AScene.hpp"
#include "protocol/UdpProtocol.hpp"

namespace client {

class GameScene : public AScene {
    public:
        static constexpr const char *Name = "game";

        explicit GameScene(SceneContext& context);
        ~GameScene() override;

        void update(double deltaTime) override;
        void render() override;

    private:
        void bindInput(Action action, server::InputKey input);
        void applySnapshot(const std::vector<EntityView>& entities);
        void setPlayerAnimation(int entity, const std::string& animation);
        int spawn(server::EntityType type, Vector2 position, std::uint8_t flags);
        void spawnExplosion(Vector2 position);
        void updateCollisionEffects(double deltaTime);
        void addCollisionEffect(const CollisionEffectView& effect);
        void playSound(std::size_t sound);

        struct CollisionFlash {
            int entity = -1;
            double elapsed = 0.0;
            std::string originalAsset;
            std::string flashAsset;
        };

        const std::unordered_map<Action, server::InputKey> _inputs;
        ClientView _view;
        std::uint8_t _keys = 0;
        std::uint32_t _tick = 0;
        std::uint32_t _lastCollisionEvent = 0;
        int _backgroundEntity = -1;
        std::unordered_map<std::uint32_t, int> _entities;
        std::unordered_map<std::uint32_t, Vector2> _previousPositions;
        std::unordered_set<int> _playersWhoPlayedIdle;
        std::vector<CollisionFlash> _collisionFlashes;
        std::vector<int> _explosions;
        std::size_t _nextShotSound = 0;
        double _shotSoundTimer = 0.0;
};

} // namespace client

#endif // CLIENT_SCENES_GAMESCENE_HPP
