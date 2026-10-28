/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Velocity.hpp
*/

#pragma once
#ifndef VELOCITY_HPP_
    #define VELOCITY_HPP_

    #include "plugins/AComponent.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Velocity : public OrByte::AComponent<OrByte::ComponentType::Velocity>
{

    Velocity() = default;
    Velocity(const Vector2& v) : velocity(v) {}

    Vector2 velocity{0.0f, 0.0f};
};

}

#endif /* VELOCITY_HPP_ */
