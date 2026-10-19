/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicSystem.cpp
*/

#include "GameEngine/ECS/Components/Orbit.hpp"
#include "GameEngine/ECS/Components/Sinus.hpp"
#include "GameEngine/ECS/Components/Spiral.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/Systems/PhysicSystem.hpp"
#include <cmath>
#include <numbers>
#include <vector>

OrByte::PhysicSystem::PhysicSystem(World& world) : _world(world)
{
    this->_chrono.reset();
}

void OrByte::PhysicSystem::_handleOrbit(Transform* transform, int entity, double deltaTime)
{
    Orbit* orbit = this->_world.getComponent<Orbit>(entity);
    if (orbit == nullptr)
        return;

    Vector2 center = orbit->target ? orbit->target->position : orbit->center;

    orbit->angle += (orbit->angleSpeed * deltaTime);
    transform->position.x = center.x + std::cos(orbit->angle) * orbit->radius;
    transform->position.y = center.y + std::sin(orbit->angle) * orbit->radius;
}

void OrByte::PhysicSystem::_handleSinus(Transform* transform, int entity, double deltaTime)
{
    Sinus* sinus = this->_world.getComponent<Sinus>(entity);
    if (sinus == nullptr)
        return;

    Vector2 center = sinus->target ? sinus->target->position : sinus->center;

    sinus->curr += (sinus->speed * deltaTime);
    transform->position.x = center.x;
    transform->position.y = center.y + std::sin(sinus->curr * std::numbers::pi * 2) * sinus->distance;
}

void OrByte::PhysicSystem::_handleSpiral(Transform* transform, int entity, double deltaTime)
{
    Spiral* spiral = this->_world.getComponent<Spiral>(entity);
    if (spiral == nullptr)
        return;

    Vector2 center = spiral->target ? spiral->target->position : spiral->center;

    spiral->angle += (spiral->angleSpeed * deltaTime);
    spiral->radius += spiral->expandSpeed * deltaTime;
    transform->position.x = center.x + std::cos(spiral->angle) * spiral->radius;
    transform->position.y = center.y + std::sin(spiral->angle) * spiral->radius;
}

void OrByte::PhysicSystem::_handleVelocity(Transform* transform, int entity, double deltaTime)
{
    Velocity* velocity = this->_world.getComponent<Velocity>(entity);
    if (velocity == nullptr)
        return;

    transform->position.x += velocity->velocity.x * deltaTime;
    transform->position.y += velocity->velocity.y * deltaTime;
}

void OrByte::PhysicSystem::update()
{
    double deltaTime = this->_chrono.getElapsedTime();
    this->_chrono.reset();

    std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity: entities) {
        Transform* transform = this->_world.getComponent<Transform>(entity);
        if (transform == nullptr)
            continue;

        this->_handleVelocity(transform, entity, deltaTime);
        this->_handleSinus(transform, entity, deltaTime);
        this->_handleOrbit(transform, entity, deltaTime);
        this->_handleSpiral(transform, entity, deltaTime);
    }
}
