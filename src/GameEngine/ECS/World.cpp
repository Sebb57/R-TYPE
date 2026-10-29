/*
** EPITECH PROJECT, 2026
** RType
** File description:
** World.cpp
*/

#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/Components/Name.hpp"

int World::createEntity() {
    int id = this->entityManager.createEntity();
    this->componentManager.addComponentToEntity<OrByte::Name>(id, "Entity_" + std::to_string(id));
    return id;

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
