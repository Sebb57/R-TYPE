#pragma once

enum class ComponentType
{
    AddHealth,
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
    Wither,
};

class IComponent
{
public:
    virtual ~IComponent() = default;
};