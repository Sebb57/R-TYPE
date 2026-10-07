/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Spiral.hpp
*/

#pragma once
#ifndef SPIRAL_HPP_
    #define SPIRAL_HPP_

    #include "IComponent.hpp"
    #include "Transform.hpp"
    #include "../Utils.hpp"

struct Spiral : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Spiral; 

    Spiral() = default;
    Spiral(const Vector2 center, const float radius, float expandSpeed, float angleSpeed)
        : center(center), radius(radius), expandSpeed(expandSpeed), angleSpeed(angleSpeed) {}
    Spiral(Transform* target, const float radius, float expandSpeed, float angleSpeed)
        : target(target), radius(radius), expandSpeed(expandSpeed), angleSpeed(angleSpeed) {}
    Spiral(const Vector2 center, Transform* target, const float radius, float expandSpeed, float angleSpeed)
        : center(center), target(target), radius(radius), expandSpeed(expandSpeed), angleSpeed(angleSpeed) {}

    Vector2 center{0.0F, 0.0F};
    Transform* target = nullptr;
    float radius = 0.0F;
    float expandSpeed = 0.0F;
    float angle = 0.0F;
    float angleSpeed = 0.0F;
};

#endif /* SPIRAL_HPP_ */
