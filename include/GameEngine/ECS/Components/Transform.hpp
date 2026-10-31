/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Transform.hpp
*/

#pragma once
#ifndef TRANSFORM_HPP_
    #define TRANSFORM_HPP_

    #include "plugins/AComponent.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Transform : public OrByte::AComponent<OrByte::ComponentType::Transform>
{

    Transform() = default;
    Transform(const Vector2& position, const Vector2& scale = { 1.f, 1.f }, float rotation = 0.f)
        : position(position), scale(scale), rotation(rotation) {}

    Vector2 position{0.0f, 0.0f};
    Vector2 scale{1.0f, 1.0f};
    float rotation{0.0f};

    void serialize(Serializer& serializer) const final
    {
        serializer.object(this->position);
        serializer.object(this->scale);
        serializer.value4b(this->rotation);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.object(this->position);
        deserializer.object(this->scale);
        deserializer.value4b(this->rotation);
    }
};

} // namespace OrByte

#endif /* TRANSFORM_HPP_ */
