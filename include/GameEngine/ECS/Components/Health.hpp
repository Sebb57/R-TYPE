/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Health.hpp
*/

#pragma once
#ifndef HEALTH_HPP_
    #define HEALTH_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

struct Health : public OrByte::AComponent<OrByte::ComponentType::Health>
{

    Health() = default;
    Health(int initHealth) : currentHealth(initHealth), maxHealth(initHealth) {}
    Health(int current, int max) : currentHealth(current), maxHealth(max)
    {
        if (this->currentHealth > this->maxHealth)
            this->currentHealth = this->maxHealth;
    }

    int currentHealth{100};
    int maxHealth{100};
};

}

#endif /* HEALTH_HPP_ */
