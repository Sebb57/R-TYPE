#pragma once

enum class ComponentType
{
    Collider,
    Health,
    Orbit,
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