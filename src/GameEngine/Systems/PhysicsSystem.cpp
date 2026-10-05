/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicsSystem.cpp
*/

#include "GameEngine/Systems/PhysicsSystem.hpp"
#include <vector>

OrByte::PhysicsSystem::PhysicsSystem(World& world) : _world(world)
{
    this->_chrono.reset();
}

void OrByte::PhysicsSystem::_handleVelocity(Transform* transform, int entity, double deltaTime)
{
    Velocity* velocity = this->_world.getComponent<Velocity>(entity);
    if (velocity == nullptr)
        return;

    transform->position.x += velocity->velocity.x * deltaTime;
    transform->position.y += velocity->velocity.y * deltaTime;
}

void OrByte::PhysicsSystem::update()
{
    double deltaTime = this->_chrono.getElapsedTime();
    this->_chrono.reset();

    std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity: entities) {
        Transform* transform = this->_world.getComponent<Transform>(entity);
        if (transform == nullptr)
            continue;

        this->_handleVelocity(transform, entity, deltaTime);
    }
}
