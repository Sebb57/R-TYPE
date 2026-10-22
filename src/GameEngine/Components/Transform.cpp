/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Transform.cpp
*/

#include "GameEngine/ECS/Components/Transform.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IComponent.hpp"

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::COMPONENT;
    }

    OrByte::IComponent *factory()
    {
        return new OrByte::Transform();
    }
}
