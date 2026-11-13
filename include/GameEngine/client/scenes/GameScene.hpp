/**
 * @file GameScene.hpp
 * @brief in game scene, feeds the local game its input and lets it render itself
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_SCENES_GAMESCENE_HPP
#define GAMEENGINE_CLIENT_SCENES_GAMESCENE_HPP

#include "GameEngine/client/scenes/AScene.hpp"
#ifdef ORBYTE_FEATURE_CHAT
    #include "GameEngine/client/scenes/ChatBox.hpp"
#endif

namespace client {

class GameScene : public AScene {
    public:
        static constexpr const char *Name = "game";

        explicit GameScene(SceneContext& context);

        void onEvent(const OrByte::Event& event) override;
        void update(double deltaTime) override;
        void render() override;

#ifdef ORBYTE_FEATURE_CHAT
    private:
        ChatBox _chat;
        server::ChatLog _chatLog;
#endif
};

} // namespace client

#endif // GAMEENGINE_CLIENT_SCENES_GAMESCENE_HPP
