/**
 * @file IComponent.hpp
 * @brief Component Plugin Interface
 *
 * @author SkyReox
 * @date 2026-10-22
**/

#pragma once
#ifndef ICOMPONENT_HPP_
    #define ICOMPONENT_HPP_

namespace OrByte {
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
        Controllable,
        Name,
    };

    class IComponent {
        public:
            [[nodiscard]] virtual ComponentType getType() const noexcept = 0;

            virtual ~IComponent() = default;
    };

} // namespace OrByte

#endif // ICOMPONENT_HPP_
