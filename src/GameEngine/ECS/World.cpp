/*
** EPITECH PROJECT, 2026
** RType
** File description:
** World.cpp
*/

#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/Components/Camera.hpp"
#include "GameEngine/ECS/Components/Name.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "constants.hpp"
#include <vector>

namespace {

constexpr const char *CameraEntityName = "Camera";

} // namespace

int World::createEntity()
{
    int id = this->entityManager.createEntity();
    this->componentManager.addComponentToEntity<OrByte::Name>(id, "Entity_" + std::to_string(id));
    return id;
}

int World::createEntity(int entityId)
{
    this->entityManager.createEntity(entityId);
    this->componentManager.addComponentToEntity<OrByte::Name>(entityId, "Entity_" + std::to_string(entityId));
    return entityId;
}

void World::destroyEntity(int entityId)
{
    const std::optional<int> holder = this->getCameraEntity();
    if (!holder || *holder != entityId) {
        this->entityManager.destroyEntity(entityId);
        this->componentManager.removeAllComponentsFromEntity(entityId);
        return;
    }

    const OrByte::Camera camera = *this->componentManager.getComponent<OrByte::Camera>(entityId);
    const auto *transform = this->componentManager.getComponent<OrByte::Transform>(entityId);
    const Vector2 position = transform != nullptr ? transform->position
        : Vector2(OrByte::constants::SERVER_WINDOW_X / 2.0F, OrByte::constants::SERVER_WINDOW_Y / 2.0F);

    this->entityManager.destroyEntity(entityId);
    this->componentManager.removeAllComponentsFromEntity(entityId);
    const int id = this->createEntity();
    this->componentManager.addComponentToEntity<OrByte::Name>(id, CameraEntityName);
    this->componentManager.addComponentToEntity<OrByte::Transform>(id, position);
    this->componentManager.addComponentToEntity<OrByte::Camera>(id, camera);
}

bool World::entityExists(int entityId) const
{
    return this->entityManager.entityExists(entityId);
}

std::vector<int> World::getExistingEntities() const
{
    return this->entityManager.getExistingEntities();
}

void World::clearWorld()
{
    for (int id : this->getExistingEntities()) {
        this->entityManager.destroyEntity(id);
        this->componentManager.removeAllComponentsFromEntity(id);
    }
    this->_selectedEntityId.reset();
}

void World::_releaseCamera(int newHolder)
{
    for (int holder : this->componentManager.getEntitiesWith(OrByte::ComponentType::Camera))
        if (holder != newHolder)
            this->componentManager.removeComponentFromEntity<OrByte::Camera>(holder);
}

int World::ensureCamera()
{
    if (const std::optional<int> holder = this->getCameraEntity())
        return *holder;

    const int id = this->createEntity();
    this->componentManager.addComponentToEntity<OrByte::Name>(id, CameraEntityName);
    this->componentManager.addComponentToEntity<OrByte::Transform>(id, Vector2(OrByte::constants::SERVER_WINDOW_X / 2.0F, OrByte::constants::SERVER_WINDOW_Y / 2.0F));
    this->componentManager.addComponentToEntity<OrByte::Camera>(id);
    return id;
}

std::optional<int> World::getCameraEntity() const
{
    const std::vector<int> holders = this->componentManager.getEntitiesWith(OrByte::ComponentType::Camera);

    if (holders.empty())
        return std::nullopt;
    return holders.front();
}

Vector2 World::getCameraOrigin(const Vector2& view)
{
    const std::optional<int> holder = this->getCameraEntity();
    if (!holder)
        return {};

    const auto *camera = this->componentManager.getComponent<OrByte::Camera>(*holder);
    const auto *transform = this->componentManager.getComponent<OrByte::Transform>(*holder);
    const Vector2 target = transform != nullptr ? transform->position : Vector2(view.x / 2.0F, view.y / 2.0F);
    return camera->origin(target, view);
}
