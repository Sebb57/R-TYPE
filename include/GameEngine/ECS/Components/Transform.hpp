/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Transform.hpp
*/

#pragma once
#ifndef TRANSFORM_HPP_
    #define TRANSFORM_HPP_

    #include "IComponent.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Transform : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Transform;

    Transform() = default;
    Transform(const Vector2& position, const Vector2& scale, float rotation)
        : position(position), scale(scale), rotation(rotation) {}

    Vector2 position{0.0f, 0.0f};
    Vector2 scale{1.0f, 1.0f};
    float rotation{0.0f};
};

}

#endif /* TRANSFORM_HPP_ */
