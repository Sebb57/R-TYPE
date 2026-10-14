#ifndef PROJECTILES_HPP
#define PROJECTILES_HPP

#include "../World.hpp"
#include <cstddef>

namespace RType {
    class Projectiles {
        public:
            Projectiles(World *world);
            ~Projectiles();
            void CreateServeurComponent(World *world);
            void CreateClientComponent(World *world);
        private:
            int ID;
    };
}

#endif
