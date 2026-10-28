/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Sprite.hpp
*/

#pragma once
#ifndef SPRITE_HPP_
    #define SPRITE_HPP_

    #include "plugins/AComponent.hpp"
    #include "GameEngine/ECS/Utils.hpp"
    #include <string>

namespace OrByte {

struct Sprite : public OrByte::AComponent<OrByte::ComponentType::Sprite>
{

    Sprite() = default;
    Sprite(const std::string& path, Vector2 size) : texturePath(path), size(size) {}

    std::string texturePath;
    Vector2 size{0.0f, 0.0f};
};

}

#endif /* SPRITE_HPP_ */
