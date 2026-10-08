/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AddHealth.hpp
*/

#pragma once
#ifndef ADDHEALTH_HPP_
    #define ADDHEALTH_HPP_

    #include "IComponent.hpp"

struct AddHealth : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::AddHealth;

    AddHealth() = default;
    AddHealth(int regenAmount) : regenAmount(regenAmount) {}

    int regenAmount{100};
};

#endif /* ADDHEALTH_HPP_ */
