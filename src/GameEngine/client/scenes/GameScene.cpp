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
#ifdef ORBYTE_FEATURE_CHAT
    #include <utility>
#endif

client::GameScene::GameScene(SceneContext& context)
    : AScene(context)
#ifdef ORBYTE_FEATURE_CHAT
    , _chat(context, [&context](std::string text) { context.game.say(std::move(text)); })
#endif
{
    this->bindAction(Action::Back, [this](bool pressed) {
        if (pressed)
            this->_context.game.leave();
    });
}

void client::GameScene::onEvent(const OrByte::Event& event)
{
#ifdef ORBYTE_FEATURE_CHAT
    if (this->_chat.onEvent(event))
        return;
#endif
    AScene::onEvent(event);
    if (this->_context.controls.actionOf(event.key) != Action::Back)
        this->_context.game.push(event);
}

void client::GameScene::update(double deltaTime)
{
    AScene::update(deltaTime);
#ifdef ORBYTE_FEATURE_CHAT
    this->_chatLog = this->_context.game.chat();
    this->_chat.update(deltaTime, this->_chatLog);
#endif
    if (!this->_context.game.running())
        this->_context.scenes.switchTo(ServerScene::Name);
}

void client::GameScene::render()
{
    AScene::render();
    this->_context.game.render();
    this->drawText(this->_context.controls.keyName(Action::Back) + "  leave", Vector2(40.0F, 1020.0F), 22, theme::Muted);
#ifdef ORBYTE_FEATURE_CHAT
    this->_chat.render(this->_chatLog);
#endif
}
