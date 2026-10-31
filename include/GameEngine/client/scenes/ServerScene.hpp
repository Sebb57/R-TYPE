/**
 * @file ServerScene.hpp
 * @brief first menu, play a solo game or browse the rooms of another server
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_SCENES_SERVERSCENE_HPP
#define GAMEENGINE_CLIENT_SCENES_SERVERSCENE_HPP

#include <string>
#include "GameEngine/client/scenes/AMenuScene.hpp"

namespace client {

class ServerScene : public AMenuScene {
    public:
        static constexpr const char *Name = "server";

        explicit ServerScene(SceneContext& context);

        void update(double deltaTime) override;

    private:
        void play();
        void connect(const std::string& host);

        std::string _address;
        std::string _username;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_SCENES_SERVERSCENE_HPP
