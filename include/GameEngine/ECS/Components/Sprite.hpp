/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Sprite.hpp
*/

#pragma once
#ifndef SPRITE_HPP_
    #define SPRITE_HPP_

    #include "GameEngine/ECS/Components/IComponent.hpp"
    #include "GameEngine/ECS/Utils.hpp"
    #include <string>

namespace OrByte {

struct Sprite : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Sprite;

    Sprite(const std::string& path, Vector2 size) : texturePath(path), size(size) {}

    std::string texturePath;
    Vector2 size;
};

}

#endif /* SPRITE_HPP_ */
