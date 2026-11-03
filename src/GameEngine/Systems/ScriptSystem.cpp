/**
 * @file ScriptSystem.cpp
 * @brief Lua script system implementation
 *
 * @author Seb
 * @date 2026-11-02
**/

#include "GameEngine/Systems/ScriptSystem.hpp"
#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/Components/Camera.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Name.hpp"
#include "GameEngine/ECS/Components/Orbit.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/Poison.hpp"
#include "GameEngine/ECS/Components/RegenHealth.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Sinus.hpp"
#include "GameEngine/ECS/Components/Spiral.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Wither.hpp"
#include "GameEngine/ECS/Components/Script.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include <algorithm>
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
    this->_bindComponents();

    this->_lua.set_function("log", [](const std::string& message) { std::cout << "[lua] " << message << std::endl; });
    this->_lua.set_function("getEntities", [this]() { return sol::as_table(this->_world.getExistingEntities()); });
    this->_lua.set_function("getComponentsFrom", [this](int id) {
        const std::vector<OrByte::IComponent*> components = this->_world.getComponents(id);
        std::vector<std::string> names;
        names.reserve(components.size());
        for (const OrByte::IComponent* component : components)
            names.emplace_back(enumToString(component->getType()));
        return sol::as_table(std::move(names));
    });
    this->_lua.set_function("getComponent", [this](int id, std::string_view name, sol::this_state ts) { return this->_pushComponent(sol::state_view(ts), id, name); });
    this->_lua.set_function("getComponentAttributes", [this](std::string_view name) {
        const auto type = _typeFromName(name);
        return sol::as_table(type ? this->_fields[static_cast<std::size_t>(*type)] : std::vector<std::string>{});
    });

    this->_assign = this->_lua.load("local obj, key, value = ...; obj[key] = value").get<sol::protected_function>();
    this->_lua.set_function("setComponentAttribute", [this](int id, std::string_view name, const std::string& field, const sol::object& value, sol::this_state ts) {
        return this->_setComponentAttribute(id, name, field, value, ts);
    });
    this->_lua.set_function("setComponentAttributeInt", [this](int id, std::string_view name, const std::string& field, int value, sol::this_state ts) {
        return this->_setComponentAttribute(id, name, field, sol::make_object(sol::state_view(ts), value), ts);
    });
    this->_lua.set_function("setComponentAttributeStr", [this](int id, std::string_view name, const std::string& field, const std::string& value, sol::this_state ts) {
        return this->_setComponentAttribute(id, name, field, sol::make_object(sol::state_view(ts), value), ts);
    });
}

sol::object OrByte::ScriptSystem::_pushComponent(sol::state_view lua, int id, std::string_view name)
{
    const auto type = _typeFromName(name);
    if (!type)
        return sol::lua_nil;
    const auto index = static_cast<std::size_t>(*type);
    IComponent* component = this->_world.getComponent(id, *type);
    if (component == nullptr || this->_pushers[index] == nullptr)
        return sol::lua_nil;
    return this->_pushers[index](std::move(lua), component);
}

bool OrByte::ScriptSystem::_setComponentAttribute(int id, std::string_view name, const std::string& field, const sol::object& value, sol::this_state ts)
{
    const sol::object component = this->_pushComponent(sol::state_view(ts), id, name);
    if (component == sol::lua_nil) {
        _reportError(id, "setComponentAttribute: no component '" + std::string(name) + "'");
        return false;
    }
    const auto& fields = this->_fields[static_cast<std::size_t>(*_typeFromName(name))];
    if (std::ranges::find(fields, field) == fields.end()) {
        _reportError(id, "setComponentAttribute: '" + std::string(name) + "' has no attribute '" + field + "'");
        return false;
    }
    auto result = this->_assign(component, field, value);
    if (!result.valid()) {
        const sol::error error = result;
        _reportError(id, "setComponentAttribute: " + std::string(name) + "." + field + ": " + error.what());
        return false;
    }
    return true;
}

std::optional<OrByte::ComponentType> OrByte::ScriptSystem::_typeFromName(std::string_view name)
{
    for (std::size_t i = 0; i < static_cast<std::size_t>(ComponentType::ENUM_END); i++)
        if (enumToString(static_cast<ComponentType>(i)) == name)
            return static_cast<ComponentType>(i);
    return std::nullopt;
}

void OrByte::ScriptSystem::_bindComponents()
{
    this->_lua.new_usertype<Vector2>("Vector2",
        sol::constructors<Vector2(), Vector2(float, float)>(),
        "x", &Vector2::x, "y", &Vector2::y,
        sol::meta_function::to_string, [](const Vector2& v) { return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")"; });
    this->_lua.new_usertype<RGBA>("RGBA", "r", &RGBA::r, "g", &RGBA::g, "b", &RGBA::b, "a", &RGBA::a);
    this->_lua.new_enum<ColliderShape>("ColliderShape", {{"Box", ColliderShape::Box}, {"Circle", ColliderShape::Circle}});
    this->_lua.new_enum<Team>("Team", {{"Player", Team::Player}, {"Enemy", Team::Enemy}});

    this->_bindComponent<Transform>("position", &Transform::position, "scale", &Transform::scale, "rotation", &Transform::rotation);
    this->_bindComponent<Velocity>("velocity", &Velocity::velocity);
    this->_bindComponent<Health>("currentHealth", &Health::currentHealth, "maxHealth", &Health::maxHealth);
    this->_bindComponent<AddHealth>("regenAmount", &AddHealth::regenAmount);
    this->_bindComponent<RemoveHealth>("hitDamage", &RemoveHealth::hitDamage);
    this->_bindComponent<RegenHealth>("regenAmount", &RegenHealth::regenAmount, "cooldown", &RegenHealth::cooldown, "duration", &RegenHealth::duration);
    this->_bindComponent<Poison>("damage", &Poison::damage, "cooldown", &Poison::cooldown, "duration", &Poison::duration);
    this->_bindComponent<Wither>("damage", &Wither::damage, "cooldown", &Wither::cooldown, "duration", &Wither::duration);
    this->_bindComponent<Collider>("size", &Collider::size, "shape", &Collider::colliderShape);
    this->_bindComponent<Owner>("team", &Owner::team);
    this->_bindComponent<Name>("name", &Name::name);
    this->_bindComponent<Camera>("boundX", &Camera::boundX, "boundY", &Camera::boundY);
    this->_bindComponent<Sprite>("texturePath", &Sprite::texturePath, "size", &Sprite::size);
    this->_bindComponent<Script>("path", sol::readonly(&Script::path));
    this->_bindComponent<Orbit>("center", &Orbit::center, "targetEntity", &Orbit::targetEntity, "radius", &Orbit::radius, "angle", &Orbit::angle, "angleSpeed", &Orbit::angleSpeed);
    this->_bindComponent<Sinus>("center", &Sinus::center, "targetEntity", &Sinus::targetEntity, "distance", &Sinus::distance, "curr", &Sinus::curr, "speed", &Sinus::speed);
    this->_bindComponent<Spiral>("center", &Spiral::center, "targetEntity", &Spiral::targetEntity, "radius", &Spiral::radius, "expandSpeed", &Spiral::expandSpeed, "angle", &Spiral::angle, "angleSpeed", &Spiral::angleSpeed);
    this->_bindComponent<Controllable>("speed", &Controllable::speed, "size", &Controllable::size, "color", &Controllable::color, "image", &Controllable::image, "boundX", &Controllable::boundX, "boundY", &Controllable::boundY);
    this->_bindComponent<Animator>("currentAnimation", &Animator::currentAnimation, "currentFrame", &Animator::currentFrame, "elapsed", &Animator::elapsed, "playing", &Animator::playing);
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
