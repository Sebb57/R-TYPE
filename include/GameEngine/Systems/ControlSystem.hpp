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
    #include <unordered_map>

namespace OrByte {

class ControlSystem : public ASystem {
    World& _world;

    std::unordered_set<Event::Key> _heldKeys;
    std::unordered_set<Event::GamepadButton> _heldButtons;
    std::unordered_map<Event::GamepadAxis, float> _gamepadAxes;

    float _inputValue(const Keybind& input) const;
    float _axis(const Keybind& positive, const Keybind& negative) const;

    public:
        ControlSystem(World& world);
        ~ControlSystem() override = default;

        void onEvent(const Event& event) override;
        void update() override;
};

} // namespace OrByte

#endif /* CONTROLSYSTEM_HPP_ */
