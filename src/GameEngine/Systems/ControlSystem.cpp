/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ControlSystem.cpp
*/

#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/Systems/ControlSystem.hpp"
#include <cmath>
#include <vector>

OrByte::ControlSystem::ControlSystem(World& world) : _world(world)
{}

float OrByte::ControlSystem::_axis(Event::Key positive, Event::Key negative) const
{
    return static_cast<float>(this->_heldKeys.contains(positive)) - static_cast<float>(this->_heldKeys.contains(negative));
}

void OrByte::ControlSystem::onEvent(const Event& event)
{
    if (event.type == Event::Type::KeyPressed)
        this->_heldKeys.insert(event.key);
    else if (event.type == Event::Type::KeyReleased)
        this->_heldKeys.erase(event.key);
}

void OrByte::ControlSystem::update()
{
    std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity: entities) {
        auto* controllable = this->_world.getComponent<Controllable>(entity);
        if (controllable == nullptr)
            continue;
        if (this->_world.getComponent<Velocity>(entity) == nullptr)
            this->_world.addComponentToEntity<Velocity>(entity);

        const Vector2 direction(this->_axis(controllable->right, controllable->left), this->_axis(controllable->down, controllable->up));
        const float length = std::hypot(direction.x, direction.y);
        const float speed = length > 0.0F ? controllable->speed / length : 0.0F;
        this->_world.getComponent<Velocity>(entity)->velocity = Vector2(direction.x * speed, direction.y * speed);
    }
}
