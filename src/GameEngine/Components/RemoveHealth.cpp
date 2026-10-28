/*
** EPITECH PROJECT, 2026
** RType
** File description:
** RemoveHealth.cpp
*/

#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IComponent.hpp"

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::COMPONENT;
    }

    OrByte::IComponent *factory()
    {
        return new OrByte::RemoveHealth();
    }
}
