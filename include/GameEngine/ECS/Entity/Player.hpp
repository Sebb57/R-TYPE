/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Player.hpp
*/

#pragma once
#ifndef PLAYER_HPP_
    #define PLAYER_HPP_

#include "../World.hpp"
#include <cstddef>

namespace OrByte {

class Player {
    public:
        Player(World *world);
        ~Player();

        void createServerComponent(World *world);
        void createClientComponent(World *world);

    private:
        int _id;
};

}

#endif /* PLAYER_HPP_ */
