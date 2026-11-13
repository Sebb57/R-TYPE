/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Poison.hpp
*/

#pragma once
#ifndef POISON_HPP_
    #define POISON_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

struct Poison : public OrByte::AComponent<OrByte::ComponentType::Poison>
{

    Poison() = default;
    Poison(int damage, float duration) : damage(damage), duration(duration) {}
    Poison(int damage, float cooldown, float duration) : damage(damage), cooldown(cooldown), duration(duration) {}

    int damage{10};
    float cooldown{1.0F};
    float duration{3.0F};

    void serialize(Serializer& serializer) const final
    {
        serializer.value4b(this->damage);
        serializer.value4b(this->cooldown);
        serializer.value4b(this->duration);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.value4b(this->damage);
        deserializer.value4b(this->cooldown);
        deserializer.value4b(this->duration);
    }
};

} // namespace OrByte

#endif /* POISON_HPP_ */
