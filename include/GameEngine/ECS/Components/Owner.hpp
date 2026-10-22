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

enum class Team { Player, Enemy };

struct Owner : public OrByte::AComponent<OrByte::ComponentType::Owner>
{

    Owner() = default;
    Owner(Team t) : team(t) {}

    Team team{Team::Player};
};

#endif /* OWNER_HPP_ */
