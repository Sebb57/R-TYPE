/*
** EPITECH PROJECT, 2026
** RType
** File description:
** HealthSystem.hpp
*/

#pragma once
#ifndef HEALTHSYSTEM_HPP_
    #define HEALTHSYSTEM_HPP_

    #include "Chronometer.hpp"
    #include "GameEngine/ECS/World.hpp"
    #include "GameEngine/ECS/Components/Health.hpp"
    #include <unordered_map>

namespace OrByte {

class HealthSystem {
    World& _world;
    std::unordered_map<int, Chronometer> _poisonCooldowns;
    std::unordered_map<int, Chronometer> _witherCooldowns;

    bool _handleDeath(Health* health, int entity);
    void _handlePoison(Health* health, int entity);
    void _handleWither(Health* health, int entity);
    void _handleRemoveHealth(Health* health, int entity);
    void _handleAddHealth(Health* health, int entity);

    public:
        HealthSystem(World& world);
        ~HealthSystem() = default;

        void update();
};

}

#endif /* HEALTHSYSTEM_HPP_ */
