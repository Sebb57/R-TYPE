#ifndef PLAYEUR_HPP
#define PLAYEUR_HPP

#include "../World.hpp"
#include <cstddef>

namespace RType {
    class Playeur {
        public:
            Playeur(World *world);
            ~Playeur();
            void CreateServeurComponent(World *world);
            void CreateClientComponent(World *world);
        private:
            int ID;
    };
}

#endif
