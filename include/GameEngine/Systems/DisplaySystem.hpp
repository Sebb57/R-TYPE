/*
** EPITECH PROJECT, 2026
** RType
** File description:
** DisplaySystem.hpp
*/

#pragma once
#ifndef DISPLAYSYSTEM_HPP_
    #define DISPLAYSYSTEM_HPP_

    #include "GameEngine/ECS/Components/Transform.hpp"
    #include "GameEngine/ECS/World.hpp"
    #include "plugins/IGraphic.hpp"

namespace OrByte {

class DisplaySystem {
    World& _world;
    IGraphic& _graphic;

    void _handleSprite(Transform* transform, int entity);

    public:
        DisplaySystem(World& world, IGraphic& graphic) : _world(world), _graphic(graphic) {}
        ~DisplaySystem() = default;

        void update();
};

}

#endif /* DISPLAYSYSTEM_HPP_ */
