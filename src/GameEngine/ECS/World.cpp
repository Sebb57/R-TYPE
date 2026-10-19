/*
** EPITECH PROJECT, 2026
** RType
** File description:
** World.cpp
*/

#include "GameEngine/ECS/World.hpp"

int World::createEntity() {
    return this->entityManager.createEntity();
}

void World::destroyEntity(int entityId) {
    this->entityManager.destroyEntity(entityId);
    this->componentManager.removeAllComponentsFromEntity(entityId);
}

bool World::entityExists(int entityId) const {
    return this->entityManager.entityExists(entityId);
}

std::vector<int> World::getExistingEntities() const {
    return this->entityManager.getExistingEntities();
}
