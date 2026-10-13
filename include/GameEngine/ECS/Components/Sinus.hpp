/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Sinus.hpp
*/

#pragma once
#ifndef SINUS_HPP_
    #define SINUS_HPP_

    #include "IComponent.hpp"
    #include "Transform.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Sinus : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Sinus;

    Sinus() = default;
    Sinus(const Vector2 center, const float distance, float speed)
        : center(center), distance(distance), speed(speed) {}
    Sinus(Transform* target, const float distance, float speed)
        : target(target), distance(distance), speed(speed) {}
    Sinus(const Vector2 center, Transform* target, const float distance, float speed)
        : center(center), target(target), distance(distance), speed(speed) {}

    Vector2 center{0.0F, 0.0F};
    Transform* target = nullptr;
    float distance = 0.0F;
    float curr = 0.0F;
    float speed = 0.0F;
};

}

#endif /* SINUS_HPP_ */
