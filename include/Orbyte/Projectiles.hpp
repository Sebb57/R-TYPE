/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Projectiles.hpp
*/

#pragma once
#ifndef PROJECTILES_HPP_
    #define PROJECTILES_HPP_

#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Utils.hpp"

namespace RType {
    class Projectiles {
        public:
            Projectiles(World *world, int shooterId, Team owner, float verticalOffset = 0.0f);
            void CreateServeurComponent(World *world);
            void CreateClientComponent(World *world);
            static void CheckCollision(World *world, Vector2 entity);
            int GetID() const { return ID; }
        private:
            int ID;
            Team _owner;
            Vector2 _spawnPos;
            Vector2 _velocity;
    };

}

#endif /* PROJECTILES_HPP_ */
