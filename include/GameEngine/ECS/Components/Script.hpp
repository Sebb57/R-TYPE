/**
 * @file Script.hpp
 * @brief Lua script attached to an entity
 * @author Seb
 * @date 2026-11-02
**/

#ifndef SCRIPT_HPP_
    #define SCRIPT_HPP_

    #include "plugins/AComponent.hpp"
    #include <string>
    #include <utility>

namespace OrByte {

struct Script : public OrByte::AComponent<OrByte::ComponentType::Script> {
    std::string path;

    Script() = default;
    explicit Script(std::string path) : path(std::move(path)) {}

    void serialize(Serializer& serializer) const final { serializer.text1b(this->path, OrByte::MaxStringSize); }
    void deserialize(Deserializer& deserializer) final { deserializer.text1b(this->path, OrByte::MaxStringSize); }
};

} // namespace OrByte

#endif /* SCRIPT_HPP_ */
