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
    #include "plugins/ASystem.hpp"
    #include "GameEngine/ECS/Components/Health.hpp"
    #include <unordered_map>

namespace OrByte {

class HealthSystem : public ASystem {
    World& _world;
    std::unordered_map<int, Chronometer> _poisonCooldowns;
    std::unordered_map<int, Chronometer> _witherCooldowns;
    std::unordered_map<int, Chronometer> _regenCooldowns;

    bool _handleDeath(Health* health, int entity);
    void _handleRemoveHealth(Health* health, int entity);
    void _handlePoison(Health* health, int entity);
    void _handleWither(Health* health, int entity);
    void _handleAddHealth(Health* health, int entity);
    void _handleRegenHealth(Health* health, int entity);

    public:
        HealthSystem(World& world);
        ~HealthSystem() override = default;

        void update() override;
};

} // namespace OrByte

#endif /* HEALTHSYSTEM_HPP_ */
