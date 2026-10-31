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

    void serialize(Serializer& serializer) const final
    {
        serializer.value4b(this->currentHealth);
        serializer.value4b(this->maxHealth);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.value4b(this->currentHealth);
        deserializer.value4b(this->maxHealth);
    }
};

}

#endif /* HEALTH_HPP_ */
