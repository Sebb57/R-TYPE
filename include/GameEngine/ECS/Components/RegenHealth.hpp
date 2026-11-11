/*
** EPITECH PROJECT, 2026
** RType
** File description:
** RegenHealth.hpp
*/

#pragma once
#ifndef REGENHEALTH_HPP_
    #define REGENHEALTH_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

struct RegenHealth : public OrByte::AComponent<OrByte::ComponentType::RegenHealth>
{

    RegenHealth() = default;
    RegenHealth(int regenAmount, float duration) : regenAmount(regenAmount), duration(duration) {}
    RegenHealth(int regenAmount, float cooldown, float duration) : regenAmount(regenAmount), cooldown(cooldown), duration(duration) {}

    int regenAmount{100};
    float cooldown{1.0F};
    float duration{3.0F};

    void serialize(Serializer& serializer) const final
    {
        serializer.value4b(this->regenAmount);
        serializer.value4b(this->cooldown);
        serializer.value4b(this->duration);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.value4b(this->regenAmount);
        deserializer.value4b(this->cooldown);
        deserializer.value4b(this->duration);
    }
};

} // namespace OrByte

#endif /* REGENHEALTH_HPP_ */
