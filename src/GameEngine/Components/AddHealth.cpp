/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AddHealth.cpp
*/

#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IComponent.hpp"

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::COMPONENT;
    }

    OrByte::IComponent *factory()
    {
        return new OrByte::AddHealth();
    }
}
