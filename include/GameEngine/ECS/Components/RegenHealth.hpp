/*
** EPITECH PROJECT, 2026
** RType
** File description:
** RegenHealth.hpp
*/

#pragma once
#ifndef REGENHEALTH_HPP_
    #define REGENHEALTH_HPP_

    #include "IComponent.hpp"

namespace OrByte {

struct RegenHealth : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::RegenHealth;

    RegenHealth() = default;
    RegenHealth(int regenAmount, float duration) : regenAmount(regenAmount), duration(duration) {}
    RegenHealth(int regenAmount, float cooldown, float duration) : regenAmount(regenAmount), cooldown(cooldown), duration(duration) {}

    int regenAmount{100};
    float cooldown{1.0F};
    float duration{3.0F};
};

}

#endif /* REGENHEALTH_HPP_ */
