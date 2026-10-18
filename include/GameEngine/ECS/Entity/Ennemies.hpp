#ifndef ENNEMIES_HPP
#define ENNEMIES_HPP

#include "../World.hpp"
#include <cstddef>

namespace RType {
    class Ennemies {
        public:
            Ennemies(World *world);
            ~Ennemies();
            void CreateServeurComponent(World *world);
            void CreateClientComponent(World *world);
            void DestructEnnemies(World *world);
        private:
            int ID;
            float RandomLine;
    };
}

#endif
