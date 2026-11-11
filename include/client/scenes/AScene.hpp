/**
 * @file AScene.hpp
 * @brief base scene with its own ecs world, systems and action bindings
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_ASCENE_HPP
#define CLIENT_SCENES_ASCENE_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "GameEngine/ECS/World.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"
#include "client/Controls.hpp"
#include "client/scenes/IScene.hpp"
#include "client/scenes/SceneContext.hpp"

namespace client {

class AScene : public IScene {
    public:
        explicit AScene(SceneContext& context);

        void onEvent(const OrByte::Event& event) override;
        void update(double deltaTime) override;
        void render() override;

    protected:
        using ActionHandler = std::function<void(bool pressed)>;

        void bindAction(Action action, ActionHandler handler);
        void drawText(const std::string& text, Vector2 position, unsigned size = 24, OrByte::RGBA color = {255, 255, 255}) const;
        void drawPanel(Vector2 position, Vector2 size, OrByte::RGBA fill, unsigned outlineThickness = 0, OrByte::RGBA outline = {0, 0, 0}) const;

        SceneContext& _context;
        World _world;

    private:
        OrByte::DynamicLibrary _physicsLoader;
        OrByte::DynamicLibrary _displayLoader;
        std::unique_ptr<OrByte::ISystem> _physics;
        std::unique_ptr<OrByte::ISystem> _display;
        std::unordered_map<Action, ActionHandler> _actions;
        std::unordered_set<OrByte::Event::Key> _heldKeys;
};

} // namespace client

#endif // CLIENT_SCENES_ASCENE_HPP
