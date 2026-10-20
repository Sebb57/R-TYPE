/*
** EPITECH PROJECT, 2026
** RType
** File description:
** main.cpp
*/

#include "constants.hpp"
#include "GameEngine/GameEngine.hpp"
#include <iostream>

int main(void)
{
    try {
        OrByte::GameEngine engine;

        // Run
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return OrByte::constants::FAILURE;
    }

    return OrByte::constants::SUCCESS;
}
