/**
 * @file ScriptSystem.cpp
 * @brief Lua script system implementation
 *
 * @author Seb
 * @date 2026-11-02
**/

#include "GameEngine/Systems/ScriptSystem.hpp"
#include "GameEngine/ECS/ComponentPlugins.hpp"
#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/Components/Camera.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Music.hpp"
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
#include <system_error>
#include <tuple>
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
        const Script* script = this->_world.getActiveComponent<Script>(entity);
        if (script == nullptr || !this->_exists(entity))
            continue;

        this->_sync(entity, script->path);

        auto it = this->_instances.find(entity);
        if (it != this->_instances.end() && it->second.valid && this->_exists(entity))
            this->_call(entity, it->second, deltaTime);
        if (this->_pendingScene)
            break;
    }
    this->_flushDestroy();
    this->_flushScene();
}

void OrByte::ScriptSystem::onCollisions(const std::vector<std::pair<int, int>>& pairs)
{
    std::set<std::pair<int, int>> current;
    for (const auto& [first, second] : pairs)
        if (first != second && this->_exists(first) && this->_exists(second))
            current.emplace(std::min(first, second), std::max(first, second));
    for (const auto& pair : current) {
        if (this->_contacts.contains(pair))
            continue;
        for (const auto& [entity, other] : {pair, std::pair{pair.second, pair.first}}) {
            if (!this->_exists(entity) || !this->_exists(other))
                continue;
            const Script* script = this->_world.getActiveComponent<Script>(entity);
            if (!script)
                continue;
            this->_sync(entity, script->path);
            auto it = this->_instances.find(entity);
            if (it == this->_instances.end() || !it->second.valid || !this->_exists(entity) || !this->_exists(other))
                continue;
            const auto callback = it->second.env.raw_get<sol::object>("onCollisionEnter");
            if (callback.get_type() != sol::type::function)
                continue;
            auto result = callback.as<sol::protected_function>()(entity, other);
            if (!result.valid()) {
                const sol::error error = result;
                _reportError(entity, error.what());
                it->second.valid = false;
            }
            if (this->_pendingScene)
                break;
        }
        if (this->_pendingScene)
            break;
    }
    this->_contacts = std::move(current);
    this->_flushDestroy();
    this->_flushScene();
}

void OrByte::ScriptSystem::_flushScene()
{
    if (!this->_pendingScene)
        return;
    // All callbacks have returned; release their environments before the old world.
    this->_instances.clear();
    this->_contacts.clear();
    this->_world = std::move(*this->_pendingScene);
    this->_pendingScene.reset();
    this->_chrono.reset();
}

void OrByte::ScriptSystem::_flushDestroy()
{
    // Keep existing raw component references alive until all Lua callbacks return.
    for (int entity : this->_pendingDestroy)
        this->_world.destroyEntity(entity);
    this->_pendingDestroy.clear();
}

void OrByte::ScriptSystem::_collect()
{
    std::erase_if(this->_instances, [this](const auto& entry) {
        const auto& [entity, instance] = entry;
        const Script* script = this->_world.getActiveComponent<Script>(entity);
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
    this->_lua.set_function("getEntities", [this]() {
        auto entities = this->_world.getExistingEntities();
        std::erase_if(entities, [this](int entity) { return !this->_exists(entity); });
        return sol::as_table(std::move(entities));
    });
    this->_lua.set_function("getComponentsFrom", [this](int id) {
        const std::vector<OrByte::IComponent*> components = this->_world.getActiveComponents(id);
        std::vector<std::string> names;
        names.reserve(components.size());
        if (this->_exists(id))
            for (const OrByte::IComponent* component : components)
                names.emplace_back(enumToString(component->getType()));
        return sol::as_table(std::move(names));
    });
    this->_lua.set_function("getComponent", [this](int id, std::string_view name, sol::this_state ts) { return this->_pushComponent(sol::state_view(ts), id, name); });
    this->_lua.set_function("getComponentAttributes", [this](std::string_view name) {
        const auto type = _typeFromName(name);
        return sol::as_table(type ? this->_fields[static_cast<std::size_t>(*type)] : std::vector<std::string>{});
    });

    this->_lua.set_function("destroyEntity", [this](int id) {
        if (!this->_exists(id))
            return false;
        this->_pendingDestroy.insert(id);
        return true;
    });
    this->_lua.set_function("loadScene", [this](const std::string& path) -> std::tuple<bool, sol::optional<std::string>> {
        if (this->_pendingScene)
            return {false, std::string("A scene load is already pending")};
        try {
            if (std::filesystem::path(path).extension() != WORLD_EXTENSION)
                return {false, std::string("Expected an .obe scene")};
            // Keep plugin factories alive while their components exist in the world.
            static const auto components = loadComponentPlugins("plugins/components");
            auto scene = std::make_unique<World>();
            scene->setActiveComponents(this->_world.getActiveComponentTypes());
            deserializeWorld(*scene, path, components.factories);
            this->_pendingScene = std::move(scene);
            return {true, sol::nullopt};
        } catch (const std::exception& error) {
            return {false, std::string(error.what())};
        }
    });
    this->_lua.set_function("duplicateEntity", [this](int source) -> sol::optional<int> {
        if (!this->_exists(source))
            return sol::nullopt;
        std::vector<std::unique_ptr<IComponent>> copies;
        for (const IComponent* component : this->_world.getActiveComponents(source)) {
            const auto type = component->getType();
            const auto index = static_cast<std::size_t>(type);
            // Camera is exclusive in World; adding a second one would remove the original.
            if (type == ComponentType::Camera || index >= this->_cloners.size() || !this->_cloners[index]) {
                _reportError(source, "duplicateEntity: component cannot be copied: " + std::string(enumToString(type)));
                return sol::nullopt;
            }
            copies.push_back(this->_cloners[index](*component));
        }
        const int duplicate = this->_world.createEntity();
        for (auto& component : copies)
            this->_world.addComponentToEntity(duplicate, std::move(component));
        return duplicate;
    });
    this->_lua.set_function("createEntity", [this]() { return this->_world.createEntity(); });
    this->_lua.set_function("addComponent", [this](int id, std::string_view name, sol::optional<sol::table> init, sol::this_state ts) {
        return this->_addComponent(id, name, init, ts);
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
    if (!type || !this->_exists(id))
        return sol::lua_nil;
    const auto index = static_cast<std::size_t>(*type);
    IComponent* component = this->_world.getActiveComponent(id, *type);
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

sol::object OrByte::ScriptSystem::_addComponent(int id, std::string_view name, const sol::optional<sol::table>& init, sol::this_state ts)
{
    if (!this->_exists(id)) {
        _reportError(id, "addComponent: entity does not exist");
        return sol::lua_nil;
    }
    const auto type = _typeFromName(name);
    if (type && !this->_world.isComponentActive(*type))
        return sol::lua_nil;
    if (!type || this->_adders[static_cast<std::size_t>(*type)] == nullptr) {
        _reportError(id, "addComponent: unknown component '" + std::string(name) + "'");
        return sol::lua_nil;
    }

    // never replace an existing component: Lua may still hold a reference to it
    if (this->_world.getActiveComponent(id, *type) == nullptr)
        this->_adders[static_cast<std::size_t>(*type)](this->_world, id);

    if (init) {
        for (const auto& [key, value] : *init) {
            if (key.get_type() != sol::type::string) {
                _reportError(id, "addComponent: '" + std::string(name) + "' init keys must be attribute names");
                continue;
            }
            this->_setComponentAttribute(id, name, key.as<std::string>(), value, ts);
        }
    }
    return this->_pushComponent(sol::state_view(ts), id, name);
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
    this->_lua.new_enum<MusicState>("MusicState", {{"Playing", MusicState::Playing}, {"Paused", MusicState::Paused}, {"Stopped", MusicState::Stopped}});

    this->_bindComponent<Transform>("position", &Transform::position, "scale", &Transform::scale, "rotation", &Transform::rotation);
    this->_bindComponent<Velocity>("velocity", &Velocity::velocity);
    this->_bindComponent<Health>("currentHealth", &Health::currentHealth, "maxHealth", &Health::maxHealth);
    this->_bindComponent<Music>("path", &Music::path, "volume", &Music::volume, "pitch", &Music::pitch, "loop", &Music::loop, "state", &Music::state);
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

    void onCollisions(OrByte::ISystem& system, const std::vector<std::pair<int, int>>& pairs)
    {
        static_cast<OrByte::ScriptSystem&>(system).onCollisions(pairs);
    }

    OrByte::ISystem *factory(World& world)
    {
        return new OrByte::ScriptSystem(world);
    }
}
