/**
 * @file ScriptSystem.cpp
 * @brief Lua script system implementation
 *
 * @author Seb
 * @date 2026-11-02
**/

#include "GameEngine/Systems/ScriptSystem.hpp"
#include "GameEngine/ECS/Components/Script.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"
#include <iostream>
#include <system_error>
#include <utility>
#include <vector>

OrByte::ScriptSystem::ScriptSystem(World& world, std::filesystem::path scriptsDir)
    : _world(world), _scriptsDir(std::move(scriptsDir))
{
    this->_lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
    this->_bindApi();
    this->_chrono.reset();
}

void OrByte::ScriptSystem::update()
{
    const double deltaTime = this->_chrono.getElapsedTime();
    this->_chrono.reset();
    this->_collect();

    const std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity : entities) {
        const Script* script = this->_world.getComponent<Script>(entity);
        if (script == nullptr)
            continue;

        this->_sync(entity, script->path);

        auto it = this->_instances.find(entity);
        if (it != this->_instances.end() && it->second.valid)
            this->_call(entity, it->second, deltaTime);
    }
}

void OrByte::ScriptSystem::_collect()
{
    std::erase_if(this->_instances, [this](const auto& entry) {
        const auto& [entity, instance] = entry;
        const Script* script = this->_world.getComponent<Script>(entity);
        return script == nullptr || script->path != instance.path;
    });
}

void OrByte::ScriptSystem::_sync(int entity, const std::string& path)
{
    std::error_code ec;
    const auto lastWrite = std::filesystem::last_write_time(this->_resolve(path), ec);

    auto it = this->_instances.find(entity);
    if (it != this->_instances.end() && (ec || it->second.lastWrite == lastWrite))
        return;

    Instance instance;
    try {
        instance = this->_load(path);
    } catch (const ScriptException& e) {
        this->_reportError(entity, e.what());
        instance.path = path;
        instance.valid = false;
    }
    instance.lastWrite = ec ? std::filesystem::file_time_type{} : lastWrite;
    this->_instances.insert_or_assign(entity, std::move(instance));
}

OrByte::ScriptSystem::Instance OrByte::ScriptSystem::_load(const std::string& path)
{
    Instance instance;
    instance.path = path;
    instance.env = sol::environment(this->_lua, sol::create, this->_lua.globals());

    auto result = this->_lua.safe_script_file(this->_resolve(path).string(), instance.env, sol::script_pass_on_error);
    if (!result.valid()) {
        const sol::error error = result;
        throw ScriptLoadFailed(path, error.what());
    }

    const auto update = instance.env.raw_get<sol::object>(UPDATE);
    if (update.get_type() != sol::type::function)
        throw MissingUpdate(path);

    instance.update = update.as<sol::protected_function>();
    instance.valid = true;
    return instance;
}

void OrByte::ScriptSystem::_call(int entity, Instance& instance, double deltaTime)
{
    auto result = instance.update(entity, deltaTime);
    if (result.valid())
        return;

    const sol::error error = result;
    _reportError(entity, error.what());
    instance.valid = false;
}

void OrByte::ScriptSystem::_bindApi()
{
    this->_lua.set_function("log", [](const std::string& message) { std::cout << "[lua] " << message << std::endl; });

    // TODO: add usefull engine binds
}

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::SYSTEMS;
    }

    OrByte::ISystem *factory(World& world)
    {
        return new OrByte::ScriptSystem(world);
    }
}
