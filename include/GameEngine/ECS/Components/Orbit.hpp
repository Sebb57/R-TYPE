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
    Orbit(Transform* target, const float radius, float angleSpeed)
        : target(target), radius(radius), angleSpeed(angleSpeed) {}
    Orbit(const Vector2 center, Transform* target, const float radius, float angleSpeed)
        : center(center), target(target), radius(radius), angleSpeed(angleSpeed) {}

    Vector2 center{0.0F, 0.0F};
    Transform* target = nullptr;
    float radius = 0.0F;
    float angle = 0.0F;
    float angleSpeed = 0.0F;
};

}

#endif /* ORBIT_HPP_ */
