/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ColliderSystem.hpp
*/

#pragma once
#ifndef COLLIDERSYSTEM_HPP_
    #define COLLIDERSYSTEM_HPP_

    #include "GameEngine/ECS/Components/Transform.hpp"
    #include "GameEngine/ECS/World.hpp"
    #include "GameEngine/ECS/Components/Collider.hpp"
    #include <vector>

namespace OrByte {

struct AABB {
    float left;
    float right;
    float top;
    float bottom;
};

struct Circle {
    Vector2 center;
    float radius;
};

struct CollisionEntity {
    int entity;
    Collider* collider;
    Transform* transform;
    AABB box;
};

class ColliderSystem {
    World& _world;
    
    bool checkCollision(Collider* colliderA, Collider* colliderB, Transform* transformA, Transform* transformB);
    AABB getAABB(const Collider& collider, const Transform& transform);
    Circle getCircle(const Collider& collider, const Transform& transform);
    bool intersects(const Circle& a, const Circle& b);
    bool intersects(const Circle& circle, const AABB& box);
    bool intersects(const AABB& a, const AABB& b);
    
    public:
        ColliderSystem(World& world);
        ~ColliderSystem() = default;

        std::vector<std::pair<int, int>> update();
};

}

#endif /* COLLIDERSYSTEM_HPP_ */