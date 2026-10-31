/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Owner.hpp
*/

#pragma once
#ifndef OWNER_HPP_
    #define OWNER_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

enum class Team { Player, Enemy };

struct Owner : public OrByte::AComponent<OrByte::ComponentType::Owner>
{

    Owner() = default;
    Owner(Team t) : team(t) {}

    Team team{Team::Player};

    void serialize(OrByte::Serializer& serializer) const final { serializer.value4b(this->team); }
    void deserialize(OrByte::Deserializer& deserializer) final { deserializer.value4b(this->team); }
};

} // namespace OrByte

#endif /* OWNER_HPP_ */
