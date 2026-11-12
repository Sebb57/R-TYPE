/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicSystem.cpp
*/

#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Orbit.hpp"
#include "GameEngine/ECS/Components/Sinus.hpp"
#include "GameEngine/ECS/Components/Spiral.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/Systems/PhysicSystem.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"
#include <cmath>
#include <numbers>
#include <vector>

OrByte::PhysicSystem::PhysicSystem(World& world) : _world(world)
{
    this->_chrono.reset();
}

void OrByte::PhysicSystem::_handleOrbit(Transform* transform, int entity, double deltaTime)
{
    Orbit* orbit = this->_world.getActiveComponent<Orbit>(entity);
    if (orbit == nullptr)
        return;

    Transform* target = this->_world.getActiveComponent<Transform>(orbit->targetEntity);
    Vector2 center = target ? target->position : orbit->center;

    orbit->angle += (orbit->angleSpeed * deltaTime);
    transform->position.x = center.x + std::cos(orbit->angle) * orbit->radius;
    transform->position.y = center.y + std::sin(orbit->angle) * orbit->radius;
}

void OrByte::PhysicSystem::_handleSinus(Transform* transform, int entity, double deltaTime)
{
    Sinus* sinus = this->_world.getActiveComponent<Sinus>(entity);
    if (sinus == nullptr)
        return;

    Transform* target = this->_world.getActiveComponent<Transform>(sinus->targetEntity);
    Vector2 center = target ? target->position : sinus->center;

    sinus->curr += (sinus->speed * deltaTime);
    transform->position.x = center.x;
    transform->position.y = center.y + std::sin(sinus->curr * std::numbers::pi * 2) * sinus->distance;
}

void OrByte::PhysicSystem::_handleSpiral(Transform* transform, int entity, double deltaTime)
{
    Spiral* spiral = this->_world.getActiveComponent<Spiral>(entity);
    if (spiral == nullptr)
        return;

    Transform* target = this->_world.getActiveComponent<Transform>(spiral->targetEntity);
    Vector2 center = target ? target->position : spiral->center;

    spiral->angle += (spiral->angleSpeed * deltaTime);
    spiral->radius += spiral->expandSpeed * deltaTime;
    transform->position.x = center.x + std::cos(spiral->angle) * spiral->radius;
    transform->position.y = center.y + std::sin(spiral->angle) * spiral->radius;
}

void OrByte::PhysicSystem::_handleVelocity(Transform* transform, int entity, double deltaTime)
{
    Velocity* velocity = this->_world.getActiveComponent<Velocity>(entity);
    if (velocity == nullptr)
        return;

    transform->position.x += velocity->velocity.x * deltaTime;
    transform->position.y += velocity->velocity.y * deltaTime;
}

void OrByte::PhysicSystem::_handleBounds(Transform* transform, int entity)
{
    const Controllable* controllable = this->_world.getActiveComponent<Controllable>(entity);
    if (controllable == nullptr)
        return;

    transform->position.x = clampToBound(transform->position.x, controllable->size.x / 2.0F, controllable->boundX);
    transform->position.y = clampToBound(transform->position.y, controllable->size.y / 2.0F, controllable->boundY);
}

void OrByte::PhysicSystem::update()
{
    double deltaTime = this->_chrono.getElapsedTime();
    this->_chrono.reset();

    std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity: entities) {
        Transform* transform = this->_world.getActiveComponent<Transform>(entity);
        if (transform == nullptr)
            continue;

        this->_handleVelocity(transform, entity, deltaTime);
        this->_handleSinus(transform, entity, deltaTime);
        this->_handleOrbit(transform, entity, deltaTime);
        this->_handleSpiral(transform, entity, deltaTime);
        this->_handleBounds(transform, entity);
    }
}


extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::SYSTEMS;
    }

    OrByte::ISystem *factory(World& world)
    {
        return new OrByte::PhysicSystem(world);
    }
}
