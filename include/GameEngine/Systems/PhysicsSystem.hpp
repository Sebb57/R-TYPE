/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicsSystem.hpp
*/

#pragma once
#ifndef PHYSICSSYSTEM_HPP_
    #define PHYSICSSYSTEM_HPP_

    #include "World.hpp"

namespace Orbyte {


class PhysicsSystem {
    World& _world;

    public:
        PhysicsSystem(World& world) : _world(world) {}
        ~PhysicsSystem() = default;

        void update();
};

}

#endif /* PHYSICSSYSTEM_HPP_ */
