/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Owner.hpp
*/

#pragma once
#ifndef OWNER_HPP_
    #define OWNER_HPP_

    #include "IComponent.hpp"

enum class Team { Player, Enemy };

struct Owner : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Owner;

    Owner() = default;
    Owner(Team t) : team(t) {}

    Team team{Team::Player};
};

#endif /* OWNER_HPP_ */
