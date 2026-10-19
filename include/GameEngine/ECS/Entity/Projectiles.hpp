#ifndef PROJECTILES_HPP
#define PROJECTILES_HPP

#include "../World.hpp"
#include <cstddef>

namespace RType {
    class Projectiles {
        public:
            Projectiles(World *world, int shooterId, Team owner);
            ~Projectiles();
            void CreateServeurComponent(World *world);
            void CreateClientComponent(World *world);
            void CheckCollision(World *world, Vector2 entity);
            int GetID() const { return ID; }
        private:
            int ID;
            Team _owner;
            Vector2 _spawnPos;
            Vector2 _velocity;
    };
}

#endif
