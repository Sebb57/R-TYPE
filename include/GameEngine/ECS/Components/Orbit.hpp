/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Orbit.hpp
*/

#pragma once
#ifndef ORBIT_HPP_
    #define ORBIT_HPP_

    #include "plugins/AComponent.hpp"
    #include "Transform.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Orbit : public OrByte::AComponent<OrByte::ComponentType::Orbit>
{

    Orbit() = default;
    Orbit(const Vector2 center, const float radius, float angleSpeed)
        : center(center), radius(radius), angleSpeed(angleSpeed) {}
    Orbit(int targetEntity, const float radius, float angleSpeed)
        : targetEntity(targetEntity), radius(radius), angleSpeed(angleSpeed) {}
    Orbit(const Vector2 center, int targetEntity, const float radius, float angleSpeed)
        : center(center), targetEntity(targetEntity), radius(radius), angleSpeed(angleSpeed) {}

    Vector2 center{960.0F, 540.0F};
    int targetEntity = -1;
    float radius = 100.0F;
    float angle = 0.0F;
    float angleSpeed = 1.0F;

    void serialize(Serializer& serializer) const final
    {
        serializer.object(this->center);
        serializer.value4b(this->targetEntity);
        serializer.value4b(this->radius);
        serializer.value4b(this->angle);
        serializer.value4b(this->angleSpeed);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.object(this->center);
        deserializer.value4b(this->targetEntity);
        deserializer.value4b(this->radius);
        deserializer.value4b(this->angle);
        deserializer.value4b(this->angleSpeed);
    }
};

} // namespace OrByte

#endif /* ORBIT_HPP_ */
