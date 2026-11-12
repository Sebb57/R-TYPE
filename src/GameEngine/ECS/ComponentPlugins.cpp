/**
 * @file ComponentPlugins.cpp
 * @brief component plugins loading implementation
 *
 * @author Seb
 * @date 2026-10-30
**/

#include "GameEngine/ECS/ComponentPlugins.hpp"
#include "plugins/IComponent.hpp"
#include "constants.hpp"
#include <algorithm>
#include <functional>
#include <utility>

namespace {

OrByte::ComponentPlugins loadPlugins(const std::filesystem::path& directory, const std::function<bool(const std::string&)>& accept)
{
    OrByte::ComponentPlugins plugins;

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        // Linux and Windows builds share plugins/: skip the other platform's libraries
        if (!entry.is_regular_file() || entry.path().extension() != OrByte::constants::PLUGIN_EXTENSION)
            continue;
        const std::string name = entry.path().stem().string();
        if (!accept(name))
            continue;

        auto library = std::make_unique<OrByte::DynamicLibrary>(entry.path());
        if (library->type() != OrByte::DynamicLibrary::PluginType::COMPONENT)
            continue;

        auto factory = library->loadFunction<OrByte::ComponentFactory>("factory");
        const std::unique_ptr<OrByte::IComponent> component{factory()};

        plugins.factories[component->getType()] = factory;
        plugins.names[component->getType()] = name;
        plugins.loaders.push_back(std::move(library));
    }
    return plugins;
}

} // namespace

OrByte::ComponentPlugins OrByte::loadComponentPlugins(const std::filesystem::path& directory)
{
    return loadPlugins(directory, [](const std::string&) { return true; });
}

OrByte::ComponentPlugins OrByte::loadComponentPlugins(const std::filesystem::path& directory, const std::vector<std::string>& enabled)
{
    return loadPlugins(directory, [&enabled](const std::string& name) { return std::ranges::contains(enabled, name); });
}
