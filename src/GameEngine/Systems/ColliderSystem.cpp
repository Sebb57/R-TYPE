/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ColliderSystem.cpp
*/

#include "GameEngine/Systems/ColliderSystem.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"

using namespace OrByte;

ColliderSystem::ColliderSystem(World& world)
    : _world(world)
{
}

AABB ColliderSystem::getAABB(const Collider& collider, const Transform& transform) {
    const float width = collider.size.x * transform.scale.x;
    const float height = collider.size.y * transform.scale.y;

    return AABB(
        transform.position.x - (width / 2.0F),
        transform.position.x + (width / 2.0F),
        transform.position.y - (height / 2.0F),
        transform.position.y + (height / 2.0F)
    );
}

Circle ColliderSystem::getCircle(const Collider& collider, const Transform& transform) {
    const float radius = (collider.size.x * transform.scale.x) / 2.0F;

    return Circle(
        {transform.position.x, transform.position.y},
        radius
    );
}

bool ColliderSystem::intersects(const AABB& a, const AABB& b){
    return !(a.left >= b.right || a.right <= b.left || a.top >= b.bottom || a.bottom <= b.top);
}

bool ColliderSystem::intersects(const Circle& a, const Circle& b) {
    const float dx = a.center.x - b.center.x;
    const float dy = a.center.y - b.center.y;
    const float distanceSquared = (dx * dx) + (dy * dy);
    const float radiusSum = a.radius + b.radius;

    return distanceSquared <= (radiusSum * radiusSum);
}

bool ColliderSystem::intersects(const Circle& circle, const AABB& box) {
    const float closestX = std::max(box.left, std::min(circle.center.x, box.right));
    const float closestY = std::max(box.top, std::min(circle.center.y, box.bottom));

    const float dx = circle.center.x - closestX;
    const float dy = circle.center.y - closestY;

    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

bool ColliderSystem::checkCollision(Collider* colliderA, Collider* colliderB, Transform* transformA, Transform* transformB) {
    if (!colliderA || !colliderB || !transformA || !transformB) {
        return false;
    }

    if (colliderA->colliderShape == ColliderShape::Box && colliderB->colliderShape == ColliderShape::Box) {
        AABB boxA = getAABB(*colliderA, *transformA);
        AABB boxB = getAABB(*colliderB, *transformB);
        return intersects(boxA, boxB);
    } else if (colliderA->colliderShape == ColliderShape::Circle && colliderB->colliderShape == ColliderShape::Circle) {
        Circle circleA = getCircle(*colliderA, *transformA);
        Circle circleB = getCircle(*colliderB, *transformB);
        return intersects(circleA, circleB);
    } else if (colliderA->colliderShape == ColliderShape::Circle && colliderB->colliderShape == ColliderShape::Box) {
        Circle circle = getCircle(*colliderA, *transformA);
        AABB box = getAABB(*colliderB, *transformB);
        return intersects(circle, box);
    } else if (colliderA->colliderShape == ColliderShape::Box && colliderB->colliderShape == ColliderShape::Circle) {
        AABB box = getAABB(*colliderA, *transformA);
        Circle circle = getCircle(*colliderB, *transformB);
        return intersects(circle, box);
    }

    return false;
}

void ColliderSystem::update()
{
    std::vector<std::pair<int, int>> collisions;
    std::vector<CollisionEntity> collidables;

    const auto entities = _world.getExistingEntities();

    collidables.reserve(entities.size());

    for (int entityID : entities) {
        auto* collider = this->_world.getActiveComponent<Collider>(entityID);
        auto* transform = this->_world.getActiveComponent<Transform>(entityID);

        if (collider == nullptr || transform == nullptr) {
            continue;
        }

        AABB box{};

        if (collider->colliderShape == ColliderShape::Circle) {
            Circle circle = getCircle(*collider, *transform);

            box = AABB(
                circle.center.x - circle.radius,
                circle.center.x + circle.radius,
                circle.center.y - circle.radius,
                circle.center.y + circle.radius
            );
        } else {
            box = getAABB(*collider, *transform);
        }

        collidables.push_back({
            entityID,
            collider,
            transform,
            box
        });
    }

    for (size_t i = 0; i < collidables.size(); ++i) {
        for (size_t j = i + 1; j < collidables.size(); ++j) {
            const CollisionEntity& a = collidables[i];
            const CollisionEntity& b = collidables[j];

            if (!intersects(a.box, b.box)) {
                continue;
            }

            if (checkCollision(
                a.collider,
                b.collider,
                a.transform,
                b.transform
            )) {
                collisions.emplace_back(a.entity, b.entity);
            }
        }
    }
    this->_collisions = std::move(collisions);
}


extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::SYSTEMS;
    }

    OrByte::ISystem *factory(World& world)
    {
        return new OrByte::ColliderSystem(world);
    }
}
