/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Sinus.hpp
*/

#pragma once
#ifndef SINUS_HPP_
    #define SINUS_HPP_

    #include "plugins/AComponent.hpp"
    #include "Transform.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Sinus : public OrByte::AComponent<OrByte::ComponentType::Sinus>
{

    Sinus() = default;
    Sinus(const Vector2 center, const float distance, float speed)
        : center(center), distance(distance), speed(speed) {}
    Sinus(int targetEntity, const float distance, float speed)
        : targetEntity(targetEntity), distance(distance), speed(speed) {}
    Sinus(const Vector2 center, int targetEntity, const float distance, float speed)
        : center(center), targetEntity(targetEntity), distance(distance), speed(speed) {}

    Vector2 center{960.0F, 540.0F};
    int targetEntity = -1;
    float distance = 50.0F;
    float curr = 0.0F;
    float speed = 0.5F;

    void serialize(Serializer& serializer) const final
    {
        serializer.object(this->center);
        serializer.value4b(this->targetEntity);
        serializer.value4b(this->distance);
        serializer.value4b(this->curr);
        serializer.value4b(this->speed);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.object(this->center);
        deserializer.value4b(this->targetEntity);
        deserializer.value4b(this->distance);
        deserializer.value4b(this->curr);
        deserializer.value4b(this->speed);
    }
};

} // namespace OrByte

#endif /* SINUS_HPP_ */
