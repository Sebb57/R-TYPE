/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Sinus.cpp
*/

#include "GameEngine/ECS/Components/Sinus.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IComponent.hpp"

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::COMPONENT;
    }

    OrByte::IComponent *factory()
    {
        return new OrByte::Sinus();
    }
}
