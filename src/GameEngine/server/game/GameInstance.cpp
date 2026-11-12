/**
 * @file GameInstance.cpp
 * @brief solo game simulation implementation
 *
 * @author Seb
 * @date 2026-10-12
**/

#include "GameEngine/server/game/GameInstance.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <utility>
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/client/GamePlugins.hpp"
#include "constants.hpp"
#include "plugins/IAudio.hpp"
#include "plugins/IGraphic.hpp"

namespace {

constexpr std::string_view SYSTEM_SUFFIX = "System";

constexpr std::array<std::string_view, 6> TICK_ORDER{"Control", "Script", "Physic", "Collider", "Health", "Audio"};

[[nodiscard]] std::filesystem::path systemPath(const std::string& name)
{
    return std::filesystem::path(OrByte::constants::SYSTEMS_DIR) / OrByte::constants::pluginFile(name + std::string(SYSTEM_SUFFIX));
}

[[nodiscard]] std::vector<std::string> systemsOnDisk()
{
    std::vector<std::string> names;
    std::error_code error;

    for (const auto& entry : std::filesystem::directory_iterator(OrByte::constants::SYSTEMS_DIR, error)) {
        const std::string stem = entry.path().stem().string();

        if (entry.path().extension() == OrByte::constants::PLUGIN_EXTENSION && stem.ends_with(SYSTEM_SUFFIX) && stem.size() > SYSTEM_SUFFIX.size())
            names.push_back(stem.substr(0, stem.size() - SYSTEM_SUFFIX.size()));
    }
    return names;
}

[[nodiscard]] std::size_t tickRank(const std::string& name)
{
    return static_cast<std::size_t>(std::ranges::distance(TICK_ORDER.begin(), std::ranges::find(TICK_ORDER, name)));
}

} // namespace

server::GameInstance::GameInstance(OrByte::IGraphic& graphic, unsigned tickRate, unsigned snapshotRate, const Setup& setup)
    : _tickRate(std::max(tickRate, 1U))
    , _snapshotRate(std::max(snapshotRate, 1U))
    , _world(std::make_unique<World>())
{
    if (const auto& compiled = client::compiledComponents(); compiled) {
        std::vector<OrByte::ComponentType> active;
        for (std::size_t i = 0; i < static_cast<std::size_t>(OrByte::ComponentType::ENUM_END); ++i) {
            const auto type = static_cast<OrByte::ComponentType>(i);
            if (std::ranges::contains(*compiled, enumToString(type)))
                active.push_back(type);
        }
        this->_world->setActiveComponents(std::move(active));
    }
    this->loadSystems(graphic);
    if (setup)
        setup(*this->_world);
}

void server::GameInstance::loadSystems(OrByte::IGraphic& graphic)
{
    const auto& compiled = client::compiledSystems();
    std::vector<std::string> names = compiled ? *compiled : systemsOnDisk();

    std::ranges::stable_sort(names, {}, tickRank);
    for (const std::string& name : names) {
        try {
            auto loader = std::make_unique<OrByte::DynamicLibrary>(systemPath(name));
            OrByte::ISystem *system = this->createSystem(name, *loader, graphic);

            if (system == nullptr)
                continue;
            LoadedSystem loaded{name, std::move(loader), std::unique_ptr<OrByte::ISystem>(system)};
            if (name == "Display")
                this->_display = std::move(loaded);
            else
                this->_systems.push_back(std::move(loaded));
        } catch (const std::exception& error) {
            std::cerr << "[Game] " << name << " system disabled: " << error.what() << std::endl;
        }
    }
}

OrByte::ISystem *server::GameInstance::createSystem(const std::string& name, OrByte::DynamicLibrary& loader, OrByte::IGraphic& graphic)
{
    if (name == "Display")
        return loader.loadFunction<OrByte::ISystem* (*)(World&, OrByte::IGraphic&)>("factory")(*this->_world, graphic);
    if (name == "Audio")
        return this->loadAudio() ? loader.loadFunction<OrByte::ISystem* (*)(World&, OrByte::IAudio&)>("factory")(*this->_world, *this->_audio) : nullptr;
    if (name == "Script")
        this->_scriptCollisions = loader.loadFunction<decltype(_scriptCollisions)>("onCollisions");
    return loader.loadFunction<OrByte::ISystem* (*)(World&)>("factory")(*this->_world);
}

bool server::GameInstance::loadAudio()
{
    if (this->_audio)
        return true;
    try {
        this->_audioLoader = std::make_unique<OrByte::DynamicLibrary>(
            std::filesystem::path(OrByte::constants::PLUGINS_DIR) / OrByte::constants::pluginFile("SFMLAudio"));
        this->_audio.reset(this->_audioLoader->loadFunction<OrByte::IAudio* (*)()>("factory")());
    } catch (const std::exception& error) {
        std::cerr << "[Game] audio disabled: " << error.what() << std::endl;
        this->_audio.reset();
        this->_audioLoader.reset();
    }
    return this->_audio != nullptr;
}

server::GameInstance::~GameInstance()
{
    this->stop();
    this->_display.system.reset();
    for (LoadedSystem& loaded : this->_systems)
        loaded.system.reset();
    this->_world.reset();
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
    if (this->_display.system)
        this->_display.system->update();
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
        for (const LoadedSystem& loaded : this->_systems)
            loaded.system->onEvent(event);
}

void server::GameInstance::update()
{
    const auto script = std::ranges::find(this->_systems, "Script", &LoadedSystem::name);
    for (const LoadedSystem& loaded : this->_systems) {
        loaded.system->update();
        if (loaded.name == "Collider" && script != this->_systems.end() && this->_scriptCollisions)
            this->_scriptCollisions(*script->system, loaded.system->getEntityPairs());
    }
}

server::SnapshotDto server::GameInstance::makeSnapshot()
{
    SnapshotDto snapshot;
    snapshot.tick = this->_tick;
    const Vector2 origin = this->_world->getCameraOrigin(Vector2(OrByte::constants::SERVER_WINDOW_X, OrByte::constants::SERVER_WINDOW_Y));
    for (int entity : this->_world->getExistingEntities()) {
        const auto *transform = this->_world->getActiveComponent<OrByte::Transform>(entity);
        if (transform == nullptr)
            continue;
        const auto id = static_cast<std::uint32_t>(entity + 1);
        const Vector2 position(transform->position.x - origin.x, transform->position.y - origin.y);
        if (const auto *controllable = this->_world->getActiveComponent<OrByte::Controllable>(entity))
            snapshot.entities.push_back(EntityDto(id, position, transform->rotation, controllable->size, controllable->color, controllable->image));
        else if (const auto *sprite = this->_world->getActiveComponent<OrByte::Sprite>(entity))
            snapshot.entities.push_back(EntityDto(id, position, transform->rotation, sprite->size, OrByte::RGBA(255, 255, 255), sprite->texturePath));
    }
    return snapshot;
}
