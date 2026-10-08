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

struct RegenHealth : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::RegenHealth;

    RegenHealth() = default;
    RegenHealth(int regenAmount) : regenAmount(regenAmount) {}

    int regenAmount{100};
};

#endif /* REGENHEALTH_HPP_ */
