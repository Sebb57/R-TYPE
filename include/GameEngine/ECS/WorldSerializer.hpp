/**
 * @file WorldSerializer.hpp
 * @brief binary (de)serialization of a whole World to an .obe file
 *
 * @author Seb
 * @date 2026-10-29
**/

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

void serializeWorld(const World& world, const std::filesystem::path& path);
void deserializeWorld(World& world, const std::filesystem::path& path, const ComponentFactories& factories);

} // namespace OrByte

#endif /* WORLDSERIALIZER_HPP_ */
