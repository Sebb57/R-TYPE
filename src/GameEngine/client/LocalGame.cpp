/**
 * @file LocalGame.cpp
 * @brief solo game hosted by the client's own server implementation
 *
 * @author Seb
 * @date 2026-10-22
**/

#include "GameEngine/client/LocalGame.hpp"
#include <algorithm>
#include <variant>

client::LocalGame::LocalGame(server::Server& server)
    : _server(server)
    , _state(std::make_shared<State>())
{}

client::ClientView client::LocalGame::view() const
{
    std::scoped_lock lock(this->_state->mutex);
    return this->_state->view;
}

void client::LocalGame::start(const std::string& username)
{
    std::uint32_t game = 0;
    {
        std::scoped_lock lock(this->_state->mutex);
        game = this->_state->game++;
        ClientView& view = this->_state->view;
        view = ClientView();
        view.status = "Playing solo";
        view.username = username;
        view.inRoom = true;
        view.running = true;
        view.host = true;
        view.slot = 0;
        view.snapshotRate = static_cast<std::uint8_t>(std::min(this->_server.config().snapshotRate, 255U));
        view.players = {PlayerView(0, username, true, true)};
    }
    this->_server.host(username, [state = this->_state, game](server::OutboundMessage&& message) {
        receive(*state, game, message);
    });
}

void client::LocalGame::leave()
{
    {
        std::scoped_lock lock(this->_state->mutex);
        this->_state->game++;
        ClientView& view = this->_state->view;
        view.inRoom = false;
        view.running = false;
        view.players.clear();
        view.entities.clear();
        view.collisionEffects.clear();
        view.status = "Left the game";
    }
    this->_server.leave();
}

void client::LocalGame::setKeys(std::uint8_t keys)
{
    this->_server.setKeys(keys);
}

void client::LocalGame::receive(State& state, std::uint32_t game, const server::OutboundMessage& message)
{
    std::scoped_lock lock(state.mutex);
    if (game != state.game)
        return;
    ClientView& view = state.view;
    if (const auto *snapshot = std::get_if<server::SnapshotDto>(&message.payload)) {
        if (snapshot->tick <= view.tick)
            return;
        view.tick = snapshot->tick;
        view.remainingSeconds = snapshot->remainingSeconds;
        view.gameOver = snapshot->gameOver;
        view.hasGameState = true;
        view.aliveMask = snapshot->aliveMask;
        view.scores = snapshot->scores;
        view.entities.clear();
        for (const server::EntityDto& entity : snapshot->entities)
            view.entities.push_back(EntityView(entity.id, entity.type, entity.x, entity.y, entity.flags));
        return;
    }
    const auto& event = std::get<server::GameEventDto>(message.payload);
    view.collisionEffects.push_back(CollisionEffectView(event.eventId, event.entityId, event.eventType,
        static_cast<std::int16_t>(event.extra & 0xffffU), static_cast<std::int16_t>(event.extra >> 16)));
    if (view.collisionEffects.size() > 32)
        view.collisionEffects.erase(view.collisionEffects.begin());
}
