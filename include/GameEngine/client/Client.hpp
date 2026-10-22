/**
 * @file Client.hpp
 * @brief client class, owns the window, its own server, the network and the scenes
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_CLIENT_HPP
#define GAMEENGINE_CLIENT_CLIENT_HPP

#include <memory>
#include "Chronometer.hpp"
#include "GameEngine/client/ClientConfig.hpp"
#include "GameEngine/client/Controls.hpp"
#include "GameEngine/client/LocalGame.hpp"
#include "GameEngine/client/network/NetworkClient.hpp"
#include "GameEngine/client/scenes/SceneContext.hpp"
#include "GameEngine/client/scenes/SceneManager.hpp"
#include "plugins/IGraphic.hpp"
#include "GameEngine/server/Server.hpp"

namespace client {

class Client {
    public:
        explicit Client(ClientConfig config);

        void run();

    private:
        void registerScenes();
        void pollEvents();

        ClientConfig _config;
        std::unique_ptr<OrByte::IGraphic> _graphic;
        server::Server _server;
        LocalGame _game;
        NetworkClient _network;
        SceneManager _scenes;
        Controls _controls;
        SceneContext _context;
        OrByte::Chronometer _clock;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_CLIENT_HPP
