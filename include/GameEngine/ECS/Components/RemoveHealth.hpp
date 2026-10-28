/*
** EPITECH PROJECT, 2026
** RType
** File description:
** RemoveHealth.hpp
*/

#pragma once
#ifndef REMOVEHEALTH_HPP_
    #define REMOVEHEALTH_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

struct RemoveHealth : public OrByte::AComponent<OrByte::ComponentType::RemoveHealth>
{

    RemoveHealth() = default;
    RemoveHealth(int hitDamage) : hitDamage(hitDamage) {}

    int hitDamage{100};
};

}

#endif /* REMOVEHEALTH_HPP_ */
