/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Controllable.hpp
*/

#pragma once
#ifndef CONTROLLABLE_HPP_
    #define CONTROLLABLE_HPP_

    #include "plugins/AComponent.hpp"
    #include "../Utils.hpp"
    #include "GameEngine/Event.hpp"
    #include "plugins/IGraphic.hpp"
    #include <cstdint>
    #include <string>
    #include <utility>

namespace OrByte {

struct GamepadInput {
    enum class Kind : std::uint8_t {
        None,
        Button,
        Axis,
    };

    Kind kind = Kind::None;
    Event::GamepadButton button = Event::GamepadButton::Unknown;
    Event::GamepadAxis axis = Event::GamepadAxis::Unknown;
    bool positive = true;

    [[nodiscard]] static GamepadInput onButton(Event::GamepadButton button)
    {
        GamepadInput input;

        input.kind = Kind::Button;
        input.button = button;
        return input;
    }

    [[nodiscard]] static GamepadInput onAxis(Event::GamepadAxis axis, bool positive)
    {
        GamepadInput input;

        input.kind = Kind::Axis;
        input.axis = axis;
        input.positive = positive;
        return input;
    }
};

struct Keybind {
    Event::Key key = Event::Key::Unknown;
    GamepadInput pad;

    Keybind() = default;
    Keybind(Event::Key key, GamepadInput pad = {}) : key(key), pad(pad) {}

    void serialize(Serializer& serializer) const
    {
        serializer.value4b(this->key);
        serializer.value1b(this->pad.kind);
        serializer.value1b(this->pad.button);
        serializer.value1b(this->pad.axis);
        serializer.value1b(this->pad.positive);
    }

    void deserialize(Deserializer& deserializer)
    {
        deserializer.value4b(this->key);
        deserializer.value1b(this->pad.kind);
        deserializer.value1b(this->pad.button);
        deserializer.value1b(this->pad.axis);
        deserializer.value1b(this->pad.positive);
    }
};

struct Controllable : public OrByte::AComponent<OrByte::ComponentType::Controllable>
{
    Controllable() = default;
    Controllable(const Vector2& size, std::string image = "") : size(size), image(std::move(image)) {}

    Keybind up {Event::Key::Up, GamepadInput::onAxis(Event::GamepadAxis::LeftY, false)};
    Keybind down {Event::Key::Down, GamepadInput::onAxis(Event::GamepadAxis::LeftY, true)};
    Keybind left {Event::Key::Left, GamepadInput::onAxis(Event::GamepadAxis::LeftX, false)};
    Keybind right {Event::Key::Right, GamepadInput::onAxis(Event::GamepadAxis::LeftX, true)};
    float speed = 400.f;

    Vector2 size {64.f, 64.f};
    RGBA color {255, 255, 255};
    std::string image;

    float boundX = 1920.f;
    float boundY = 1080.f;

    void serialize(Serializer& serializer) const final
    {
        this->up.serialize(serializer);
        this->down.serialize(serializer);
        this->left.serialize(serializer);
        this->right.serialize(serializer);
        serializer.value4b(this->speed);
        serializer.object(this->size);
        serializer.object(this->color);
        serializer.text1b(this->image, MaxStringSize);
        serializer.value4b(this->boundX);
        serializer.value4b(this->boundY);
    }

    void deserialize(Deserializer& deserializer) final
    {
        this->up.deserialize(deserializer);
        this->down.deserialize(deserializer);
        this->left.deserialize(deserializer);
        this->right.deserialize(deserializer);
        deserializer.value4b(this->speed);
        deserializer.object(this->size);
        deserializer.object(this->color);
        deserializer.text1b(this->image, MaxStringSize);
        deserializer.value4b(this->boundX);
        deserializer.value4b(this->boundY);
    }
};

} // namespace OrByte

#endif /* CONTROLLABLE_HPP_ */
