/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicSystem.cpp
*/

#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/Systems/DisplaySystem.hpp"

void OrByte::DisplaySystem::_handleSprite(Transform* transform, int entity)
{
    Sprite* sprite = this->_world.getComponent<Sprite>(entity);
    if (sprite == nullptr)
        return;

    this->_graphic.drawImage(*transform, sprite->size, sprite->texturePath);
}

void OrByte::DisplaySystem::update()
{
    std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity: entities) {
        Transform* transform = this->_world.getComponent<Transform>(entity);
        if (transform == nullptr)
            continue;

        this->_handleSprite(transform, entity);
    }
}
