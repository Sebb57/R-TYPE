/**
 * @file ComponentPlugins.cpp
 * @brief component plugins loading implementation
 *
 * @author Seb
 * @date 2026-10-30
**/

#include "GameEngine/ECS/ComponentPlugins.hpp"
#include "plugins/IComponent.hpp"
#include <utility>

OrByte::ComponentPlugins OrByte::loadComponentPlugins(const std::filesystem::path& directory)
{
    ComponentPlugins plugins;

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        auto library = std::make_unique<DynamicLibrary>(entry.path());
        if (library->type() != DynamicLibrary::PluginType::COMPONENT)
            continue;

        auto factory = library->loadFunction<ComponentFactory>("factory");
        const std::unique_ptr<IComponent> component{factory()};

        plugins.factories[component->getType()] = factory;
        plugins.loaders.push_back(std::move(library));
    }
    return plugins;
}
