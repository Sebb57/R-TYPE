/**
 * @file Client.hpp
 * @brief client class, owns the window, its own server, the network and the scenes
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_CLIENT_HPP
#define CLIENT_CLIENT_HPP

#include <memory>
#include "Chronometer.hpp"
#include "client/ClientConfig.hpp"
#include "client/Controls.hpp"
#include "client/LocalGame.hpp"
#include "client/network/NetworkClient.hpp"
#include "client/scenes/SceneContext.hpp"
#include "client/scenes/SceneManager.hpp"
#include "plugins/IGraphic.hpp"
#include "server/Server.hpp"

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

#endif // CLIENT_CLIENT_HPP
