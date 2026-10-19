/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Projectiles.hpp
*/

#pragma once
#ifndef PROJECTILES_HPP_
    #define PROJECTILES_HPP_

#include "../World.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include <cstddef>

namespace OrByte {

class Projectiles {
    public:
        Projectiles(World *world, int shooterId, Team owner);
        ~Projectiles() = default;

        void createServerComponent(World *world);
        void createClientComponent(World *world);
        void checkCollision(World *world, Vector2 entity);
        void destructProjectiles(World *world);
        int getID() const { return this->_id; }

    private:
        int _id;
        Team _owner;
        Vector2 _spawnPos;
        Vector2 _velocity;
    };

}

#endif /* PROJECTILES_HPP_ */
