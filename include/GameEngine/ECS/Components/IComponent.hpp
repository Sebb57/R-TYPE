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
    Sinus,
    Spiral,
    Sprite,
    Transform,
    Velocity,
    Wither,
    Animator,
    Owner,
    Controllable
};

class IComponent
{
public:
    virtual ~IComponent() = default;
};