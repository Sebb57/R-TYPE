/**
 * @file SpectateScene.hpp
 * @brief spectator scene, mirrors the snapshots of another player's game into the ecs world
 *
 * @author Seb
 * @date 2026-10-23
**/

#ifndef GAMEENGINE_CLIENT_SCENES_SPECTATESCENE_HPP
#define GAMEENGINE_CLIENT_SCENES_SPECTATESCENE_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include "GameEngine/client/network/ClientView.hpp"
#include "GameEngine/client/scenes/AScene.hpp"
#ifdef ORBYTE_FEATURE_CHAT
    #include "GameEngine/client/scenes/ChatBox.hpp"
#endif

namespace client {

class SpectateScene : public AScene {
    public:
        static constexpr const char *Name = "spectate";

        explicit SpectateScene(SceneContext& context);

#ifdef ORBYTE_FEATURE_CHAT
        void onEvent(const OrByte::Event& event) override;
#endif
        void update(double deltaTime) override;
        void render() override;

    private:
        void applySnapshot();
        void freeze();
        [[nodiscard]] std::string roomName() const;
#ifdef ORBYTE_FEATURE_LAGOMETER
        void drawLagometer() const;
#endif

        ClientView _view;
        std::uint32_t _tick = 0;
        double _sinceSnapshot = 0.0;
        std::unordered_map<std::uint32_t, int> _entities;
#ifdef ORBYTE_FEATURE_CHAT
        ChatBox _chat;
#endif
};

} // namespace client

#endif // GAMEENGINE_CLIENT_SCENES_SPECTATESCENE_HPP
