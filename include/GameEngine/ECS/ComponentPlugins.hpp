/**
 * @file ComponentPlugins.hpp
 * @brief loads every component plugin and keeps them alive with their factories
 *
 * @author Seb
 * @date 2026-10-30
**/

#ifndef COMPONENTPLUGINS_HPP_
    #define COMPONENTPLUGINS_HPP_

    #include "GameEngine/ECS/WorldSerializer.hpp"
    #include "plugins/DynamicLibrary.hpp"
    #include <filesystem>
    #include <memory>
    #include <vector>

namespace OrByte {

struct ComponentPlugins {
    std::vector<std::unique_ptr<DynamicLibrary>> loaders;
    ComponentFactories factories;
};

[[nodiscard]] ComponentPlugins loadComponentPlugins(const std::filesystem::path& directory);

} // namespace OrByte

#endif /* COMPONENTPLUGINS_HPP_ */
