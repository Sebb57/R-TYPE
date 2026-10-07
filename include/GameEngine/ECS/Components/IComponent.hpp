#pragma once

enum class ComponentType
{
    Collider,
    Health,
    Orbit,
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