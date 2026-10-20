/*
** EPITECH PROJECT, 2026
** RType
** File description:
** GameEngine.hpp
*/

#pragma once
#ifndef GAMEENGINE_HPP_
    #define GAMEENGINE_HPP_

#include "plugins/DynamicLibrary.hpp"
#include "plugins/IGraphic.hpp"
#include <filesystem>
#include <memory>

namespace OrByte {

class GameEngine {
    public:
        enum GraphicalLibs {
            SFML,
        };

        GameEngine(GraphicalLibs graphicalLib = SFML);
        GameEngine(std::filesystem::path graphicPath);
        ~GameEngine() = default;

    private:
        DynamicLibrary _graphicLoader;
        std::unique_ptr<IGraphic> _graphicLib;
};

}

#endif /* GAMEENGINE_HPP_ */
