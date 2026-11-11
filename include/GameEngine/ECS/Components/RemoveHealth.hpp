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

    void serialize(Serializer& serializer) const final { serializer.value4b(this->hitDamage); }
    void deserialize(Deserializer& deserializer) final { deserializer.value4b(this->hitDamage); }
};

} // namespace OrByte

#endif /* REMOVEHEALTH_HPP_ */
