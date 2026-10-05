#pragma once

enum class ComponentType
{
    Transform,
    Sprite,
    Collider,
    Velocity,
    Health,
};

class IComponent
{
public:
    virtual ~IComponent() = default;
};