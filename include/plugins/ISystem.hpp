/**
 * @file ISystem.hpp
 * @brief System Plugin Interface
 *
 * @author SkyReox
 * @date 2026-09-15
**/

#pragma once
#ifndef ISYSTEM_HPP_
    #define ISYSTEM_HPP_

#include "GameEngine/Event.hpp"
#include <utility>
#include <vector>

namespace OrByte {
    class ISystem {
        public:
            virtual void update() = 0;
            virtual void onEvent(const Event& event) = 0;
            [[nodiscard]] virtual const std::vector<std::pair<int, int>>& getEntityPairs() const = 0;

            ISystem& operator=(const ISystem& object) = delete;
            ISystem& operator=(ISystem&& object) = delete;

            ISystem() = default;
            ISystem(const ISystem& object) = delete;
            ISystem(ISystem&& object) = delete;

            virtual ~ISystem() = default;
    };

} // namespace OrByte

#endif // ISYSTEM_HPP_
