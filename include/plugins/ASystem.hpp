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
            ASystem() = default;
            ~ASystem() override = default;
    };
} // namespace OrByte

#endif /* ASYSTEM_HPP_ */
