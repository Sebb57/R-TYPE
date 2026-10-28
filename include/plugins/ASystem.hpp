/**
 * @file ASystem.hpp
 * @brief System Plugin Abstract Class
 *
 * @author SkyReox
 * @date 2026-09-15
**/

#pragma once
#ifndef ASYSTEM_HPP_
    #define ASYSTEM_HPP_

#include "plugins/ISystem.hpp"

namespace OrByte {
    class ASystem : public ISystem {
        public:
            [[nodiscard]] virtual const std::vector<std::pair<int, int>>& getEntityPairs() const override { static const std::vector<std::pair<int, int>> empty; return empty; }

            ASystem() = default;
            ~ASystem() override = default;
    };
} // namespace OrByte

#endif /* ASYSTEM_HPP_ */
