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

    struct Name :  public OrByte::AComponent<OrByte::ComponentType::Name> {
        std::string name;

        Name() : name("Entity") {}
        Name(const std::string& name) : name(name) {}
    };

#endif /* NAME_HPP_ */
