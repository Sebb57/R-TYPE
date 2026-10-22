/**
 * @file LocalGame.hpp
 * @brief solo game hosted by the client's own server, seen as a client view
 *
 * @author Seb
 * @date 2026-10-22
**/

#ifndef GAMEENGINE_CLIENT_LOCALGAME_HPP
#define GAMEENGINE_CLIENT_LOCALGAME_HPP

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include "GameEngine/client/network/ClientView.hpp"
#include "GameEngine/server/Server.hpp"

namespace client {

class LocalGame {
    public:
        explicit LocalGame(server::Server& server);

        [[nodiscard]] ClientView view() const;
        void start(const std::string& username);
        void leave();
        void setKeys(std::uint8_t keys);

    private:
        struct State {
            std::mutex mutex;
            ClientView view;
            std::uint32_t game = 0;
        };

        static void receive(State& state, std::uint32_t game, const server::OutboundMessage& message);

        server::Server& _server;
        std::shared_ptr<State> _state;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_LOCALGAME_HPP
