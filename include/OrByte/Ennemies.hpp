#ifndef ENNEMIES_HPP
#define ENNEMIES_HPP

#include "../GameEngine/Managers/World.hpp"
#include <cstddef>

namespace RType {
    class Ennemies {
        public:
            Ennemies(World *world);
            ~Ennemies();
            CreateServeurComponent(World *world);
            CreateClientComponent(World *world)
        private:
            size_t ID;
    };
}

#endif
