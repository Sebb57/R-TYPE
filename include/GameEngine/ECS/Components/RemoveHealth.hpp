/*
** EPITECH PROJECT, 2026
** RType
** File description:
** RemoveHealth.hpp
*/

#pragma once
#ifndef REMOVEHEALTH_HPP_
    #define REMOVEHEALTH_HPP_

    #include "IComponent.hpp"

struct RemoveHealth : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::RemoveHealth;

    RemoveHealth() = default;
    RemoveHealth(int hitDamage) : hitDamage(hitDamage) {}

    int hitDamage{100};
};

#endif /* REMOVEHEALTH_HPP_ */
