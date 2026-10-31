/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ControlSystem.hpp
*/

#pragma once
#ifndef CONTROLSYSTEM_HPP_
    #define CONTROLSYSTEM_HPP_

    #include "GameEngine/ECS/World.hpp"
    #include "GameEngine/Event.hpp"
    #include "plugins/ASystem.hpp"
    #include <unordered_set>

namespace OrByte {

class ControlSystem : public ASystem {
    World& _world;
    std::unordered_set<Event::Key> _heldKeys;

    float _axis(Event::Key positive, Event::Key negative) const;

    public:
        ControlSystem(World& world);
        ~ControlSystem() override = default;

        void onEvent(const Event& event) override;
        void update() override;
};

} // namespace OrByte

#endif /* CONTROLSYSTEM_HPP_ */
