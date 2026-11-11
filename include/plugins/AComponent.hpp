/**
 * @file AComponent.hpp
 * @brief Component Plugin Abstract Class
 *
 * @author SkyReox
 * @date 2026-10-22
**/

#pragma once
#ifndef ACOMPONENT_HPP_
    #define ACOMPONENT_HPP_

#include "plugins/IComponent.hpp"

namespace OrByte {
    template <ComponentType T>
    class AComponent : public IComponent {
        public:
            inline static constexpr ComponentType Type = T;

            AComponent() = default;
            ~AComponent() override = default;

            [[nodiscard]] ComponentType getType() const noexcept override { return Type; }
    };
} // namespace OrByte

#endif /* ACOMPONENT_HPP_ */
