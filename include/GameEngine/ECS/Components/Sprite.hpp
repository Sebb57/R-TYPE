#pragma once
#include "IComponent.hpp"
#include <string>

struct Sprite : public IComponent
{
    inline static constexpr ComponentType Type = ComponentType::Sprite;

    Sprite(const std::string& path) : texturePath(path) {}

    std::string texturePath;
};