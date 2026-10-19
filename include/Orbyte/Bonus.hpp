#ifndef BONUS_HPP
#define BONUS_HPP

#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/Utils.hpp"

namespace RType {
    class Playeur;
    class Bonus {
        public:
            Bonus(World *world, bool extraShot = false);
            void CreateServeurBonus(World *world, Vector2 position);
            void CreateClientBonus(World *world, bool extraShot = false);
            int GetID() const { return ID; }
            bool GivesExtraShot() const { return ExtraShot; }
            void ApplyTo(World *world, Playeur& player) const;
        private:
            int ID;
            float RandomLine;
            bool ExtraShot;
    };
}

#endif
