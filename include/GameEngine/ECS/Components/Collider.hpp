/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Collider.hpp
*/

#pragma once
#ifndef COLLIDER_HPP_
    #define COLLIDER_HPP_

    #include "plugins/AComponent.hpp"
    #include "../Utils.hpp"

    enum class ColliderShape
    {
        Box,
        Circle,
    };
    
    
    struct Collider : public OrByte::AComponent<OrByte::ComponentType::Collider>
    {
    
        Collider() = default;
        Collider(const Vector2& size, ColliderShape shape = ColliderShape::Box) : size(size), colliderShape(shape) {}
    
        Vector2 size{0.0f, 0.0f};
        ColliderShape colliderShape{ColliderShape::Box};
    };

#endif /* COLLIDER_HPP_ */
