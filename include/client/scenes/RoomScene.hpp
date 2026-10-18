/**
 * @file RoomScene.hpp
 * @brief third menu, wait in a room until the host starts the game
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_ROOMSCENE_HPP
#define CLIENT_SCENES_ROOMSCENE_HPP

#include "client/scenes/AMenuScene.hpp"

namespace client {

class RoomScene : public AMenuScene {
    public:
        static constexpr const char *Name = "room";

        explicit RoomScene(SceneContext& context);

        void update(double deltaTime) override;
        void render() override;

    private:
        [[nodiscard]] bool canStart() const;
};

} // namespace client

#endif // CLIENT_SCENES_ROOMSCENE_HPP
