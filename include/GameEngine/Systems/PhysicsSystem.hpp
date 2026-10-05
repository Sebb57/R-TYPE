/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicsSystem.hpp
*/

#pragma once
#ifndef PHYSICSSYSTEM_HPP_
    #define PHYSICSSYSTEM_HPP_

    #include "Chronometer.hpp"
    #include "GameEngine/ECS/Components/Transform.hpp"
    #include "GameEngine/ECS/Components/Velocity.hpp"
    #include "GameEngine/ECS/World.hpp"

namespace OrByte {

class PhysicsSystem {
    Chronometer _chrono;
    World& _world;

    void _handleVelocity(Transform* transform, int entity, double deltaTime);

    public:
        PhysicsSystem(World& world);
        ~PhysicsSystem() = default;

        void update();
};

}

#endif /* PHYSICSSYSTEM_HPP_ */
