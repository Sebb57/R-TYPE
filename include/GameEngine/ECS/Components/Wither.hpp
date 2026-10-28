/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Wither.hpp
*/

#pragma once
#ifndef WITHER_HPP_
    #define WITHER_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

struct Wither : public OrByte::AComponent<OrByte::ComponentType::Wither>
{

    Wither() = default;
    Wither(int damage, float duration) : damage(damage), duration(duration) {}
    Wither(int damage, float cooldown, float duration) : damage(damage), cooldown(cooldown), duration(duration) {}

    int damage{100};
    float cooldown{1.0F};
    float duration{3.0F};
};

}

#endif /* WITHER_HPP_ */
