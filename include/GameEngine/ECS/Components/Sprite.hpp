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
    #include <string>

namespace OrByte {

struct Sprite : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Sprite;

    Sprite(const std::string& path) : texturePath(path) {}

    std::string texturePath;
};

}

#endif /* SPRITE_HPP_ */
