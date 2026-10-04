#pragma once
#include "IComponent.hpp"
#include "../Utils.hpp"

struct Velocity : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Velocity;

    Velocity() = default;
    Velocity(const Vector2& v) : velocity(v) {}

    Vector2 velocity{0.0f, 0.0f};
};