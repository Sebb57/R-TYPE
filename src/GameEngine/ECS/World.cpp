/*
** EPITECH PROJECT, 2026
** RType
** File description:
** World.cpp
*/

#include "GameEngine/ECS/World.hpp"

int World::createEntity() {
    return entityManager.createEntity();
}

void World::destroyEntity(int entityId) {
    entityManager.destroyEntity(entityId);
    componentManager.removeAllComponentsFromEntity(entityId);
}

bool World::entityExists(int entityId) const {
    return entityManager.entityExists(entityId);
}
