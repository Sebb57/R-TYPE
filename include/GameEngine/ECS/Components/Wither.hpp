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

#endif /* WITHER_HPP_ */
