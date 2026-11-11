/**
 * @file GameInstance.cpp
 * @brief solo game simulation implementation
 *
 * @author Seb
 * @date 2026-10-12
**/

#include "GameEngine/server/game/GameInstance.hpp"
#include <algorithm>
#include <chrono>
#include <utility>
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/World.hpp"
#include "constants.hpp"
#include "plugins/IGraphic.hpp"

server::GameInstance::GameInstance(OrByte::IGraphic& graphic, unsigned tickRate, unsigned snapshotRate, const Setup& setup)
    : _tickRate(std::max(tickRate, 1U))
    , _snapshotRate(std::max(snapshotRate, 1U))
    , _controlLoader("./plugins/systems/ControlSystem.so")
    , _physicLoader("./plugins/systems/PhysicSystem.so")
    , _displayLoader("./plugins/systems/DisplaySystem.so")
    , _scriptLoader("./plugins/systems/ScriptSystem.so")
    , _world(std::make_unique<World>())
    , _controlSystem(this->_controlLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
    , _physicSystem(this->_physicLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
    , _displaySystem(this->_displayLoader.loadFunction<OrByte::ISystem* (*)(World&, OrByte::IGraphic&)>("factory")(*this->_world, graphic))
    , _scriptSystem(this->_scriptLoader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world))
{
    if (setup)
        setup(*this->_world);
}

server::GameInstance::~GameInstance()
{
    this->stop();
}

void server::GameInstance::start()
{
    if (!this->_thread.joinable())
        this->_thread = std::jthread([this](std::stop_token token) { this->run(token); });
}

void server::GameInstance::stop()
{
    if (this->_thread.joinable()) {
        this->_thread.request_stop();
        this->_thread.join();
    }
    this->_finished = true;
}

bool server::GameInstance::push(const OrByte::Event& event)
{
    return this->_inbox.push(event);
}

void server::GameInstance::render()
{
    std::scoped_lock lock(this->_mutex);
    this->_displaySystem->update();
}

void server::GameInstance::setSink(SnapshotSink sink)
{
    std::scoped_lock lock(this->_mutex);
    this->_sink = std::move(sink);
}

bool server::GameInstance::finished() const
{
    return this->_finished.load();
}

void server::GameInstance::run(std::stop_token token)
{
    const auto tickDuration = std::chrono::duration<double>(1.0 / this->_tickRate);
    const unsigned snapshotInterval = std::max(this->_tickRate / this->_snapshotRate, 1U);
    auto nextTick = server::Clock::now();

    while (!token.stop_requested()) {
        nextTick += std::chrono::duration_cast<server::Clock::duration>(tickDuration);
        {
            std::scoped_lock lock(this->_mutex);
            this->applyInbound();
            this->update();
            ++this->_tick;
            if (this->_sink && this->_tick % snapshotInterval == 0)
                this->_sink(this->makeSnapshot());
        }
        std::this_thread::sleep_until(nextTick);
    }
    this->_finished = true;
}

void server::GameInstance::applyInbound()
{
    for (const OrByte::Event& event : this->_inbox.drain())
        this->_controlSystem->onEvent(event);
}

void server::GameInstance::update()
{
    this->_controlSystem->update();
    this->_scriptSystem->update();
    this->_physicSystem->update();
}

server::SnapshotDto server::GameInstance::makeSnapshot()
{
    SnapshotDto snapshot;
    snapshot.tick = this->_tick;
    const Vector2 origin = this->_world->getCameraOrigin(Vector2(OrByte::constants::SERVER_WINDOW_X, OrByte::constants::SERVER_WINDOW_Y));
    for (int entity : this->_world->getExistingEntities()) {
        const auto *transform = this->_world->getComponent<OrByte::Transform>(entity);
        if (transform == nullptr)
            continue;
        const auto id = static_cast<std::uint32_t>(entity + 1);
        const Vector2 position(transform->position.x - origin.x, transform->position.y - origin.y);
        if (const auto *controllable = this->_world->getComponent<OrByte::Controllable>(entity))
            snapshot.entities.push_back(EntityDto(id, position, transform->rotation, controllable->size, controllable->color, controllable->image));
        else if (const auto *sprite = this->_world->getComponent<OrByte::Sprite>(entity))
            snapshot.entities.push_back(EntityDto(id, position, transform->rotation, sprite->size, OrByte::RGBA(255, 255, 255), sprite->texturePath));
    }
    return snapshot;
}
