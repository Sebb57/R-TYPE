/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Orbit.cpp
*/

#include "GameEngine/ECS/Components/Orbit.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IComponent.hpp"

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::COMPONENT;
    }

    OrByte::IComponent *factory()
    {
        return new OrByte::Orbit();
    }
}
