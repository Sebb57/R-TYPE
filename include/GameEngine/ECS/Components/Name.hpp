/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Name.hpp
*/
#pragma once
#ifndef NAME_HPP_
    #define NAME_HPP_

    #include <string>
    #include "plugins/AComponent.hpp"

namespace OrByte {
    struct Name :  public OrByte::AComponent<OrByte::ComponentType::Name> {
        std::string name;

        Name() : name("Entity") {}
        Name(const std::string& name) : name(name) {}

        void serialize(OrByte::Serializer& serializer) const final { serializer.text1b(this->name, OrByte::MaxStringSize); }
        void deserialize(OrByte::Deserializer& deserializer) final { deserializer.text1b(this->name, OrByte::MaxStringSize); }
    };
} // namespace OrByte

#endif /* NAME_HPP_ */
