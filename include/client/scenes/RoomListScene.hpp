/**
 * @file RoomListScene.hpp
 * @brief second menu, list the rooms of the server, create or join one
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_ROOMLISTSCENE_HPP
#define CLIENT_SCENES_ROOMLISTSCENE_HPP

#include <vector>
#include "client/network/ClientView.hpp"
#include "client/scenes/AMenuScene.hpp"

namespace client {

class RoomListScene : public AMenuScene {
    public:
        static constexpr const char *Name = "rooms";

        explicit RoomListScene(SceneContext& context);

        void update(double deltaTime) override;

    private:
        void rebuild();

        std::vector<RoomView> _rooms;
};

} // namespace client

#endif // CLIENT_SCENES_ROOMLISTSCENE_HPP
