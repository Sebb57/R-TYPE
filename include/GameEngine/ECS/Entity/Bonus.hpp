#ifndef BONUS_HPP
#define BONUS_HPP

#include "../World.hpp"
#include <cstddef>

namespace RType {
    class Bonus {
        public:
            Bonus(World *world);
            ~Bonus();
            void CreateServeurBonus(World *world);
            void CreateClientBonus(World *world);
        private:
            int ID;
            float RandomLine;
    };
}

#endif
