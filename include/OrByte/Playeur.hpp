#ifndef PLAYEUR_HPP
#define PLAYEUR_HPP

#include "../GameEngine/ECS/World.hpp"
#include <cstddef>

namespace RType {
    class Playeur {
        public:
            Playeur(World *world);
            ~Playeur();
            CreateServeurComponent(World *world);
            CreateClientComponent(World *world);
        private:
            size_t ID;
    };
}

#endif
