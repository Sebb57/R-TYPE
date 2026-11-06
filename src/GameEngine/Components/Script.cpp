/**
 * @file Script.cpp
 * @brief script plugin factory
 *
 * @author Seb
 * @date 2026-11-02
**/

#include "GameEngine/ECS/Components/Script.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IComponent.hpp"

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::COMPONENT;
    }

    OrByte::IComponent *factory()
    {
        return new OrByte::Script();
    }
}
