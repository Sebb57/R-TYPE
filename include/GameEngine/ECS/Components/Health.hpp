#pragma once
#include "IComponent.hpp"

struct Health : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Health;

    Health() = default;
    Health(int current, int max) : currentHealth(current), maxHealth(max) {}

    int currentHealth{100};
    int maxHealth{100};
};