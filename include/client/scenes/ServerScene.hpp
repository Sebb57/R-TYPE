/**
 * @file ServerScene.hpp
 * @brief first menu, choose the server and the player name
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_SERVERSCENE_HPP
#define CLIENT_SCENES_SERVERSCENE_HPP

#include <string>
#include "client/scenes/AMenuScene.hpp"

namespace client {

class ServerScene : public AMenuScene {
    public:
        static constexpr const char *Name = "server";

        explicit ServerScene(SceneContext& context);

        void update(double deltaTime) override;

    private:
        void connect(const std::string& host);

        std::string _address;
        std::string _username;
};

} // namespace client

#endif // CLIENT_SCENES_SERVERSCENE_HPP
