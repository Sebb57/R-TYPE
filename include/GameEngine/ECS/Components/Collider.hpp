#pragma once
#include "IComponent.hpp"
#include "../Utils.hpp"

struct Collider : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Collider;

    Collider() = default;
    Collider(const Vector2& size) : size(size) {}

    Vector2 size{0.0f, 0.0f};
};