/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ColliderSystem.cpp
*/

#include "GameEngine/Systems/ColliderSystem.hpp"

using namespace OrByte;

ColliderSystem::ColliderSystem(World& world)
    : _world(world)
{
}

ColliderSystem::~ColliderSystem() = default;

AABB ColliderSystem::getAABB(const Collider& collider, const Transform& transform)
{
    const float width = collider.size.x * transform.scale.x;
    const float height = collider.size.y * transform.scale.y;

    return {
        transform.position.x - width / 2.0f,
        transform.position.x + width / 2.0f,
        transform.position.y - height / 2.0f,
        transform.position.y + height / 2.0f
    };
}

bool ColliderSystem::intersects(const AABB& a, const AABB& b)
{
    return !(a.left >= b.right || a.right <= b.left || a.top >= b.bottom || a.bottom <= b.top);
}

bool ColliderSystem::checkCollision(
    Collider* colliderA,
    Collider* colliderB,
    Transform* transformA,
    Transform* transformB)
{
    if (!colliderA || !colliderB || !transformA || !transformB) {
        return false;
    }

    const AABB boxA = getAABB(*colliderA, *transformA);
    const AABB boxB = getAABB(*colliderB, *transformB);

    return intersects(boxA, boxB);
}

std::vector<std::pair<int, int>> ColliderSystem::update()
{
    std::vector<std::pair<int, int>> collisions;

    std::vector<CollisionEntity> collidables;

    const auto entities = _world.getExistingEntities();

    collidables.reserve(entities.size());
    for (int entityID : entities) {
        Collider* collider = _world.getComponent<Collider>(entityID);
        Transform* transform = _world.getComponent<Transform>(entityID);

        if (!collider || !transform) {
            continue;
        }

        collidables.push_back({
            entityID,
            getAABB(*collider, *transform)
        });
    }

    for (size_t i = 0; i < collidables.size(); ++i) {
        for (size_t j = i + 1; j < collidables.size(); ++j) {
            if (intersects(collidables[i].box, collidables[j].box)) {
                collisions.emplace_back(
                    collidables[i].entity,
                    collidables[j].entity
                );
            }
        }
    }

    return collisions;
}