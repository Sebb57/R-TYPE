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
    #include "plugins/ASystem.hpp"

namespace OrByte {

class PhysicSystem : public ASystem {
    Chronometer _chrono;
    World& _world;

    void _handleOrbit(Transform* transform, int entity, double deltaTime);
    void _handleSinus(Transform* transform, int entity, double deltaTime);
    void _handleSpiral(Transform* transform, int entity, double deltaTime);
    void _handleVelocity(Transform* transform, int entity, double deltaTime);
    void _handleBounds(Transform* transform, int entity);

    public:
        PhysicSystem(World& world);
        ~PhysicSystem() override = default;

        void update() override;
};

} // namespace OrByte

#endif /* PHYSICSYSTEM_HPP_ */
