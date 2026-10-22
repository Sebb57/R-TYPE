/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AddHealth.hpp
*/

#pragma once
#ifndef ADDHEALTH_HPP_
    #define ADDHEALTH_HPP_

    #include "plugins/AComponent.hpp"

namespace OrByte {

struct AddHealth : public OrByte::AComponent<OrByte::ComponentType::AddHealth>
{

    AddHealth() = default;
    AddHealth(int regenAmount) : regenAmount(regenAmount) {}

    int regenAmount{100};
};

}

#endif /* ADDHEALTH_HPP_ */
