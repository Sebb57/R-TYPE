#include "World.hpp"


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

template <typename T, typename ... Args>
void World::addComponentToEntity(int entityId, Args&& ... args) {
    componentManager.addComponentToEntity<T>(entityId, std::forward<Args>(args)...);
}

template <typename T>
T* World::getComponent(int entityId) {
    return componentManager.getComponent<T>(entityId);
}

template <typename T>
void World::removeComponentFromEntity(int entityId) {
    componentManager.removeComponentFromEntity<T>(entityId);
}