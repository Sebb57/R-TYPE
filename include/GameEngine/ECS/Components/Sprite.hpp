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
    Vector2 size{64.0f, 64.0f};

    void serialize(Serializer& serializer) const final
    {
        serializer.text1b(this->texturePath, MaxStringSize);
        serializer.object(this->size);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.text1b(this->texturePath, MaxStringSize);
        deserializer.object(this->size);
    }
};

} // namespace OrByte

#endif /* SPRITE_HPP_ */
