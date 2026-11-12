/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ControlSystem.cpp
*/

#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/Systems/ControlSystem.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"
#include <cmath>
#include <vector>

OrByte::ControlSystem::ControlSystem(World& world) : _world(world)
{}

float OrByte::ControlSystem::_inputValue(const Keybind& input) const
{
    float value = this->_heldKeys.contains(input.key) ? 1.f : 0.f;

    switch (input.pad.kind) {
        case GamepadInput::Kind::Button:
            if (this->_heldButtons.contains(input.pad.button))
                value = 1.f;
            break;
        case GamepadInput::Kind::Axis: {
            auto it = this->_gamepadAxes.find(input.pad.axis);

            if (it != this->_gamepadAxes.end()) {
                constexpr float deadzone = 0.15F;
                float axisValue = it->second;

                if (std::abs(axisValue) >= deadzone) {
                    float directionalValue = input.pad.positive ? axisValue : -axisValue;

                    value = std::max(value, directionalValue);
                }
            }
            break;
        }
        case GamepadInput::Kind::None:
        default:
            break;
    }

    return value;
}

float OrByte::ControlSystem::_axis(const Keybind& positive, const Keybind& negative) const
{
    return this->_inputValue(positive) - this->_inputValue(negative);
}

void OrByte::ControlSystem::onEvent(const Event& event)
{
    switch (event.type) {
        case Event::Type::KeyPressed:
            this->_heldKeys.insert(event.key);
            break;
        case Event::Type::KeyReleased:
            this->_heldKeys.erase(event.key);
            break;
        case Event::Type::JoystickButtonPressed:
            this->_heldButtons.insert(event.gamepadButton);
            break;
        case Event::Type::JoystickButtonReleased:
            this->_heldButtons.erase(event.gamepadButton);
            break;
        case Event::Type::JoystickMoved:
            this->_gamepadAxes[event.gamepadAxis] = event.axisValue;
            break;
        case Event::Type::JoystickDisconnected:
            this->_heldButtons.clear();
            this->_gamepadAxes.clear();
            break;
        default:
            break;
    }
}

void OrByte::ControlSystem::update()
{
    std::vector<int> entities = this->_world.getExistingEntities();

    for (int entity : entities) {
        auto* controllable = this->_world.getComponent<Controllable>(entity);

        if (controllable == nullptr)
            continue;

        if (this->_world.getComponent<Velocity>(entity) == nullptr)
            this->_world.addComponentToEntity<Velocity>(entity);

        const Vector2 direction(
            this->_axis(controllable->right, controllable->left),
            this->_axis(controllable->down, controllable->up)
        );
        const float length = std::hypot(direction.x, direction.y);
        const float speed = length > 1.f ? controllable->speed / length : controllable->speed;

        this->_world.getComponent<Velocity>(entity)->velocity = Vector2(direction.x * speed, direction.y * speed);
    }
}

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::SYSTEMS;
    }

    OrByte::ISystem *factory(World& world)
    {
        return new OrByte::ControlSystem(world);
    }
}
