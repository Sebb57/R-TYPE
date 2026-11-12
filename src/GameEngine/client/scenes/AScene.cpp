/**
 * @file AScene.cpp
 * @brief base scene implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/scenes/AScene.hpp"
#include "constants.hpp"
#include <filesystem>
#include <utility>
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/client/scenes/Theme.hpp"

client::AScene::AScene(SceneContext& context)
    : _physicsLoader(std::filesystem::path(OrByte::constants::SYSTEMS_DIR) / OrByte::constants::pluginFile("PhysicSystem"))
    , _displayLoader(std::filesystem::path(OrByte::constants::SYSTEMS_DIR) / OrByte::constants::pluginFile("DisplaySystem"))
    , _context(context)
    , _physics(this->_physicsLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(this->_world))
    , _display(this->_displayLoader.loadFunction<OrByte::ISystem* (*)(World&, OrByte::IGraphic&)>("factory")(this->_world, context.graphic))
{}

void client::AScene::onEvent(const OrByte::Event& event)
{
    if (event.type != OrByte::Event::Type::KeyPressed && event.type != OrByte::Event::Type::KeyReleased)
        return;
    const bool pressed = event.type == OrByte::Event::Type::KeyPressed;
    if (pressed == this->_heldKeys.contains(event.key))
        return;
    if (pressed)
        this->_heldKeys.insert(event.key);
    else
        this->_heldKeys.erase(event.key);
    const auto action = this->_context.controls.actionOf(event.key);
    if (!action)
        return;
    const auto handler = this->_actions.find(*action);
    if (handler != this->_actions.end())
        handler->second(pressed);
}

void client::AScene::update(double)
{
    this->_physics->update();
}

void client::AScene::render()
{
    this->drawPanel(Vector2(0.0f, 0.0f), Vector2(theme::Width, theme::Height), theme::Background);
    this->_display->update();
}

void client::AScene::bindAction(Action action, ActionHandler handler)
{
    this->_actions.insert_or_assign(action, std::move(handler));
}

void client::AScene::drawText(const std::string& text, Vector2 position, unsigned size, OrByte::RGBA color) const
{
    if (this->_context.font.empty())
        return;
    this->_context.graphic.drawText(OrByte::Transform(position, Vector2(1.0f, 1.0f), 0.0f), text, size, 0.f, color, 0, {0, 0, 0}, this->_context.font);}

void client::AScene::drawPanel(Vector2 position, Vector2 size, OrByte::RGBA fill, unsigned outlineThickness, OrByte::RGBA outline) const
{
    const Vector2 center(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
    this->_context.graphic.drawRectangle(OrByte::Transform(center, Vector2(1.0f, 1.0f), 0.0f), size, fill, outlineThickness, outline);
}
