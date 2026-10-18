/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Poison.hpp
*/

#pragma once
#ifndef POISON_HPP_
    #define POISON_HPP_

    #include "IComponent.hpp"

namespace OrByte {

struct Poison : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Poison;

    Poison() = default;
    Poison(int damage, float duration) : damage(damage), duration(duration) {}
    Poison(int damage, float cooldown, float duration) : damage(damage), cooldown(cooldown), duration(duration) {}

    int damage{100};
    float cooldown{1.0F};
    float duration{3.0F};
};

}

#endif /* POISON_HPP_ */
