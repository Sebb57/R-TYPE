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

#include <cstdint>
#include "plugins/Serialization.hpp"

namespace OrByte {
    enum class ComponentType : std::uint8_t
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

        ENUM_END // no type bellow this one
    };

    class IComponent {
        public:
            [[nodiscard]] virtual ComponentType getType() const noexcept = 0;

            virtual void serialize(Serializer& serializer) const = 0;
            virtual void deserialize(Deserializer& deserializer) = 0;

            virtual ~IComponent() = default;
    };

} // namespace OrByte

#endif // ICOMPONENT_HPP_
