#ifndef ENNEMIES_HPP
#define ENNEMIES_HPP

#include "GameEngine/ECS/World.hpp"

namespace OrByte {
    class Ennemies {
        public:
            Ennemies(World *world);
            void CreateServeurComponent(World *world);
            void CreateClientComponent(World *world);
            void DestructEnnemies(World *world);
            static bool ShouldDropBonus();
            int GetID() const { return ID; }
        private:
            int ID;
            float RandomLine;
    };
}

#endif
