#ifndef PLAYEUR_HPP
#define PLAYEUR_HPP

#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include <cstdint>
#include <vector>

namespace OrByte {
    class Playeur {
        public:
            Playeur(World *world);
            void CreateServeurComponent(World *world, Vector2 position, std::uint8_t playerSlot);
            void CreateClientComponent(World *world, std::uint8_t playerSlot);
            void Steer(World *world, Vector2 direction);
            std::vector<int> Fire(World *world);
            void ApplyBonus(World *world, bool extraShot);
            void ReceiveEnemyHit(World *world, int damage);
            void KeepInBounds(World *world, float width, float height);
            int GetID() const { return ID; }
        private:
            int ID;
            unsigned ProjectileCount{1};
    };
}

#endif
