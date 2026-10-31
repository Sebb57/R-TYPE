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
    #include <exception>
    #include <filesystem>
    #include <string>
    #include <unordered_map>
    #include <utility>

namespace OrByte {

using ComponentFactory = IComponent* (*)();
using ComponentFactories = std::unordered_map<ComponentType, ComponentFactory>;

inline constexpr std::uint32_t WORLD_MAGIC = 0x69676967;
inline constexpr const char *WORLD_EXTENSION = ".obe";

class WorldSerializerException : public std::exception {
    public:
        explicit WorldSerializerException(std::string msg) : _msg(std::move(msg)) {}
        [[nodiscard]] const char *what() const noexcept override { return this->_msg.c_str(); }

    private:
        std::string _msg;
};

class WorldOpenFailed : public WorldSerializerException {
    public:
        explicit WorldOpenFailed(const std::filesystem::path& path)
            : WorldSerializerException("Cannot open " + path.string()) {}
};

class WorldWriteFailed : public WorldSerializerException {
    public:
        explicit WorldWriteFailed(const std::filesystem::path& path)
            : WorldSerializerException("Cannot write " + path.string()) {}
};

class InvalidWorld : public WorldSerializerException {
    public:
        InvalidWorld(const std::filesystem::path& path, const std::string& reason)
            : WorldSerializerException(path.string() + " " + reason) {}
};

class MissingComponent : public WorldSerializerException {
    public:
        MissingComponent(const std::filesystem::path& path, ComponentType type)
            : WorldSerializerException(path.string() + " uses component type " + std::to_string(static_cast<int>(type)) + " which is not loaded") {}
};

void serializeWorld(const World& world, const std::filesystem::path& path);
void deserializeWorld(World& world, const std::filesystem::path& path, const ComponentFactories& factories);

} // namespace OrByte

#endif /* WORLDSERIALIZER_HPP_ */
