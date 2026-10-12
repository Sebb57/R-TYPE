/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicSystem.hpp
*/

#pragma once
#ifndef PHYSICSYSTEM_HPP_
    #define PHYSICSYSTEM_HPP_

    #include "Chronometer.hpp"
    #include "GameEngine/ECS/Components/Transform.hpp"
    #include "GameEngine/ECS/World.hpp"

namespace OrByte {

class PhysicSystem {
    Chronometer _chrono;
    World& _world;

    void _handleOrbit(Transform* transform, int entity, double deltaTime);
    void _handleVelocity(Transform* transform, int entity, double deltaTime);

    public:
        PhysicSystem(World& world);
        ~PhysicSystem() = default;

        void update();
};

}

#endif /* PHYSICSYSTEM_HPP_ */
