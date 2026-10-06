/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Collider.hpp
*/

#pragma once
#ifndef COLLIDER_HPP_
    #define COLLIDER_HPP_

    #include "IComponent.hpp"
    #include "../Utils.hpp"
    
    struct Collider : public IComponent
    {
        inline static constexpr ComponentType Type = ComponentType::Collider;
    
        Collider() = default;
        Collider(const Vector2& size) : size(size) {}
    
        Vector2 size{0.0f, 0.0f};
    };

#endif /* COLLIDER_HPP_ */
