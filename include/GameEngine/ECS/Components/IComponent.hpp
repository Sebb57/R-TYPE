#pragma once

enum class ComponentType
{
    Collider,
    Health,
    Orbit,
    Sprite,
    Transform,
    Velocity,
};

class IComponent
{
public:
    virtual ~IComponent() = default;
};