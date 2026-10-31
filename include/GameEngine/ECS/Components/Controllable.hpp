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
    #include <string>
    #include <utility>

namespace OrByte {

struct Controllable : public OrByte::AComponent<OrByte::ComponentType::Controllable>
{

    Controllable() = default;
    Controllable(const Vector2& size, std::string image = "") : size(size), image(std::move(image)) {}

    Event::Key up{Event::Key::Up};
    Event::Key down{Event::Key::Down};
    Event::Key left{Event::Key::Left};
    Event::Key right{Event::Key::Right};
    float speed{400.0f};

    Vector2 size{64.0f, 64.0f};
    RGBA color{255, 255, 255};
    std::string image;

    void serialize(Serializer& serializer) const final
    {
        serializer.value4b(this->up);
        serializer.value4b(this->down);
        serializer.value4b(this->left);
        serializer.value4b(this->right);
        serializer.value4b(this->speed);
        serializer.object(this->size);
        serializer.object(this->color);
        serializer.text1b(this->image, MaxStringSize);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.value4b(this->up);
        deserializer.value4b(this->down);
        deserializer.value4b(this->left);
        deserializer.value4b(this->right);
        deserializer.value4b(this->speed);
        deserializer.object(this->size);
        deserializer.object(this->color);
        deserializer.text1b(this->image, MaxStringSize);
    }
};

} // namespace OrByte

#endif /* CONTROLLABLE_HPP_ */
