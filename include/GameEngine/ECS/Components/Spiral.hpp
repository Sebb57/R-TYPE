/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Spiral.hpp
*/

#pragma once
#ifndef SPIRAL_HPP_
    #define SPIRAL_HPP_

    #include "plugins/AComponent.hpp"
    #include "Transform.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Spiral : public OrByte::AComponent<OrByte::ComponentType::Spiral>
{

    Spiral() = default;
    Spiral(const Vector2 center, const float radius, float expandSpeed, float angleSpeed)
        : center(center), radius(radius), expandSpeed(expandSpeed), angleSpeed(angleSpeed) {}
    Spiral(int targetEntity, const float radius, float expandSpeed, float angleSpeed)
        : targetEntity(targetEntity), radius(radius), expandSpeed(expandSpeed), angleSpeed(angleSpeed) {}
    Spiral(const Vector2 center, int targetEntity, const float radius, float expandSpeed, float angleSpeed)
        : center(center), targetEntity(targetEntity), radius(radius), expandSpeed(expandSpeed), angleSpeed(angleSpeed) {}

    Vector2 center{0.0F, 0.0F};
    int targetEntity = -1;
    float radius = 0.0F;
    float expandSpeed = 0.0F;
    float angle = 0.0F;
    float angleSpeed = 0.0F;

    void serialize(Serializer& serializer) const final
    {
        serializer.object(this->center);
        serializer.value4b(this->targetEntity);
        serializer.value4b(this->radius);
        serializer.value4b(this->expandSpeed);
        serializer.value4b(this->angle);
        serializer.value4b(this->angleSpeed);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.object(this->center);
        deserializer.value4b(this->targetEntity);
        deserializer.value4b(this->radius);
        deserializer.value4b(this->expandSpeed);
        deserializer.value4b(this->angle);
        deserializer.value4b(this->angleSpeed);
    }
};

} // namespace OrByte

#endif /* SPIRAL_HPP_ */
