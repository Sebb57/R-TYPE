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
#include <string>
#include <unordered_map>
#include <vector>
#include "GameEngine/ECS/Components/Sprite.hpp"
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
        void addSprite(server::EntityType type, const std::string& path, Vector2 size);
        void applySnapshot(const std::vector<EntityView>& entities);
        int spawn(server::EntityType type, Vector2 position);

        const std::unordered_map<Action, server::InputKey> _inputs;
        ClientView _view;
        std::uint8_t _keys = 0;
        std::uint32_t _tick = 0;
        std::unordered_map<std::uint32_t, int> _entities;
        std::unordered_map<server::EntityType, OrByte::Sprite> _sprites;
};

} // namespace client

#endif // CLIENT_SCENES_GAMESCENE_HPP
