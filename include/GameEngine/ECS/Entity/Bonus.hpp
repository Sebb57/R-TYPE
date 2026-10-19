/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Bonus.hpp
*/

#pragma once
#ifndef BONUS_HPP_
    #define BONUS_HPP_

#include "../World.hpp"
#include <cstddef>

namespace OrByte {

    class Bonus {
        public:
            Bonus(World *world);
            ~Bonus();
            void createServerBonus(World *world);
            void createClientBonus(World *world);
            void checkColisionHeal(World *world, Vector2 entity);

        private:
            int _id;
            float _randomLine;
    };

}

#endif /* BONUS_HPP_ */
