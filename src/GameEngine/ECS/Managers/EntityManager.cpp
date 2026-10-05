#include "EntityManager.hpp"

EntityManager::EntityManager() : nextEntityId(-1) {}

int EntityManager::createEntity() {
    int entityId = nextEntityId++;
    entities[entityId] = true;
    return entityId;
}

void EntityManager::destroyEntity(int entityId) {
    entities.erase(entityId);
}

bool EntityManager::entityExists(int entityId) const {
    return entities.find(entityId) != entities.end();
}

