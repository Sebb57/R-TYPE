#pragma once

enum class ComponentType
{
    Collider,
    Health,
    Orbit,
    Poison,
    RegenHealth,
    RemoveHealth,
    Spiral,
    Sprite,
    Transform,
    Velocity,
};

class IComponent
{
public:
    virtual ~IComponent() = default;
};