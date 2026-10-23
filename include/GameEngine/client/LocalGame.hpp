/**
 * @file LocalGame.hpp
 * @brief solo game computed and rendered locally, hosted by the client's own server for spectators
 *
 * @author Seb
 * @date 2026-10-22
**/

#ifndef GAMEENGINE_CLIENT_LOCALGAME_HPP
#define GAMEENGINE_CLIENT_LOCALGAME_HPP

#include <memory>
#include <string>
#include "GameEngine/Event.hpp"
#include "GameEngine/server/Server.hpp"
#include "GameEngine/server/game/GameInstance.hpp"
#include "plugins/IGraphic.hpp"

namespace client {

class LocalGame {
    public:
        LocalGame(server::Server& server, OrByte::IGraphic& graphic, server::GameInstance::Setup setup);
        ~LocalGame();

        LocalGame(const LocalGame&) = delete;
        LocalGame& operator=(const LocalGame&) = delete;
        LocalGame(LocalGame&&) = delete;
        LocalGame& operator=(LocalGame&&) = delete;

        void start(const std::string& username);
        void leave();
        void push(const OrByte::Event& event);
        void render();
        [[nodiscard]] bool running() const;

    private:
        server::Server& _server;
        OrByte::IGraphic& _graphic;
        server::GameInstance::Setup _setup;
        std::shared_ptr<server::GameInstance> _game;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_LOCALGAME_HPP
