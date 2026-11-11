/**
 * @file IScene.hpp
 * @brief scene interface
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_SCENES_ISCENE_HPP
#define GAMEENGINE_CLIENT_SCENES_ISCENE_HPP

#include "GameEngine/Event.hpp"

namespace client {

class IScene {
    public:
        virtual void onEvent(const OrByte::Event& event) = 0;
        virtual void update(double deltaTime) = 0;
        virtual void render() = 0;

        IScene& operator=(const IScene& object) = delete;
        IScene& operator=(IScene&& object) = delete;

        IScene() = default;
        IScene(const IScene& object) = delete;
        IScene(IScene&& object) = delete;

        virtual ~IScene() = default;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_SCENES_ISCENE_HPP
