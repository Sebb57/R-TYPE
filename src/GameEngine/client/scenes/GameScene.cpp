/**
 * @file GameScene.cpp
 * @brief in game scene implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/scenes/GameScene.hpp"
#include "GameEngine/client/LocalGame.hpp"
#include "GameEngine/client/scenes/SceneManager.hpp"
#include "GameEngine/client/scenes/ServerScene.hpp"
#include "GameEngine/client/scenes/Theme.hpp"

client::GameScene::GameScene(SceneContext& context) : AScene(context)
{
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.game.leave();
    });
}

void client::GameScene::onEvent(const OrByte::Event& event)
{
    AScene::onEvent(event);
    if (this->_context.controls.actionOf(event.key) != Action::Back)
        this->_context.game.push(event);
}

void client::GameScene::update(double deltaTime)
{
    AScene::update(deltaTime);
    if (!this->_context.game.running())
        this->_context.scenes.switchTo(ServerScene::Name);
}

void client::GameScene::render()
{
    AScene::render();
    this->_context.game.render();
    this->drawText(this->_context.controls.keyName(Action::Back) + "  leave", Vector2(40.0F, 1020.0F), 22, theme::Muted);
}
