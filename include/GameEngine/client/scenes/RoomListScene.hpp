/**
 * @file RoomListScene.hpp
 * @brief browse menu, list the rooms hosted by another server
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_SCENES_ROOMLISTSCENE_HPP
#define GAMEENGINE_CLIENT_SCENES_ROOMLISTSCENE_HPP

#include <vector>
#include "GameEngine/client/network/ClientView.hpp"
#include "GameEngine/client/scenes/AMenuScene.hpp"

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

#endif // GAMEENGINE_CLIENT_SCENES_ROOMLISTSCENE_HPP
