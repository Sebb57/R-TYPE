/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Detail.hpp
*/

#ifndef DETAIL_HPP
#define DETAIL_HPP

#include "GameEngine/ECS/Components/IComponent.hpp"
#include <vector>

namespace OrByte {
    class Detail {
        public:
            Detail(World *world, int id);
            ~Detail() = default;
            void affichageDetail(World *world, int id);
        private:
            std::vector<ComponentType> _componentUse;

    };
}

#endif