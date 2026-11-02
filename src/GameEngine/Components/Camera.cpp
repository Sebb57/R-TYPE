/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Camera.cpp
*/

#include "GameEngine/ECS/Components/Camera.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IComponent.hpp"

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::COMPONENT;
    }

    OrByte::IComponent *factory()
    {
        return new OrByte::Camera();
    }
}
