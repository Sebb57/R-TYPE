/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AddHealth.hpp
*/

#pragma once
#ifndef ADDHEALTH_HPP_
    #define ADDHEALTH_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

struct AddHealth : public OrByte::AComponent<OrByte::ComponentType::AddHealth>
{

    AddHealth() = default;
    AddHealth(int regenAmount) : regenAmount(regenAmount) {}

    int regenAmount{100};

    void serialize(Serializer& serializer) const final { serializer.value4b(this->regenAmount); }
    void deserialize(Deserializer& deserializer) final { deserializer.value4b(this->regenAmount); }
};

} // namespace OrByte

#endif /* ADDHEALTH_HPP_ */
