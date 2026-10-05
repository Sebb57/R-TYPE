#ifndef PROJECTILES_HPP
#define PROJECTILES_HPP

#include "../GameEngine/Managers/World.hpp"
#include <cstddef>

namespace RType {
    class Projectiles {
        public:
            Projectiles(World *world);
            ~Projectiles();
            CreateServeurComponent(World *world);
            CreateClientComponent(World *world);
        private:
            size_t ID;
    };
}

#endif
