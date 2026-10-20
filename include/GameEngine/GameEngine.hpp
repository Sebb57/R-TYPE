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
#include <vector>

namespace OrByte {

class GameEngine {
    public:
        enum GraphicalLibs {
            SFML,
        };

        GameEngine(GraphicalLibs graphicalLib = SFML);
        GameEngine(std::filesystem::path graphicPath);
        ~GameEngine() = default;

        void run();

    private:
        DynamicLibrary _graphicLoader;
        std::unique_ptr<IGraphic> _graphicLib;
        std::vector<Event> _events;

        bool _handleEvent(Event& event);
        void _handleEvents();
        void _update();
        void _render();
};

}

#endif /* GAMEENGINE_HPP_ */
