/**
 * @file WorldSerializer.hpp
 * @brief binary (de)serialization of a whole World to an .obe file
 *
 * @author Seb
 * @date 2026-10-29
**/

#pragma once
#ifndef WORLDSERIALIZER_HPP_
    #define WORLDSERIALIZER_HPP_

    #include "GameEngine/ECS/World.hpp"
    #include "plugins/IComponent.hpp"
    #include <cstdint>
    #include <filesystem>
    #include <unordered_map>

namespace OrByte {

using ComponentFactory = IComponent* (*)();
using ComponentFactories = std::unordered_map<ComponentType, ComponentFactory>;

inline constexpr std::uint32_t WORLD_MAGIC = 0x69676967;
inline constexpr const char *WORLD_EXTENSION = ".obe";

/**
 * @brief write every entity of the world and their components to path
 * @throw std::runtime_error if the file cannot be written
 */
void serializeWorld(const World& world, const std::filesystem::path& path);

/**
 * @brief replace the world content with the one stored in path
 * @throw std::runtime_error if the file cannot be read, is not a valid world
 *        or uses a component type that has no factory
 */
void deserializeWorld(World& world, const std::filesystem::path& path, const ComponentFactories& factories);

} // namespace OrByte

#endif /* WORLDSERIALIZER_HPP_ */
