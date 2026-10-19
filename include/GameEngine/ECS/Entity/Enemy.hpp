/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Enemies.hpp
*/

#pragma once
#ifndef ENEMY_HPP_
    #define ENEMY_HPP_

#include "../World.hpp"
#include <cstddef>

namespace OrByte {

class Enemy {
    public:
        Enemy(World *world);
        ~Enemy() = default;

        void createServerComponent(World *world);
        void createClientComponent(World *world);
        void destructEnemy(World *world);

    private:
        int _id;
        float _randomLine;
};

}

#endif /* ENEMY_HPP_ */
