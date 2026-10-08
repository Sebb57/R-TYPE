/*
** EPITECH PROJECT, 2026
** RType
** File description:
** HealthSystem.hpp
*/

#pragma once
#ifndef HEALTHSYSTEM_HPP_
    #define HEALTHSYSTEM_HPP_

    #include "GameEngine/ECS/World.hpp"
    #include "GameEngine/ECS/Components/Health.hpp"

namespace OrByte {

class HealthSystem {
    World& _world;

    bool _handleDeath(Health* health, int entity);
    void _handleRegenHealth(Health* health, int entity);
    void _handleRemoveHealth(Health* health, int entity);

    public:
        HealthSystem(World& world);
        ~HealthSystem() = default;

        void update();
};

}

#endif /* HEALTHSYSTEM_HPP_ */
