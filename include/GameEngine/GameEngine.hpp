/*
** EPITECH PROJECT, 2026
** RType
** File description:
** GameEngine.hpp
*/

#pragma once
#ifndef GAMEENGINE_HPP_
    #define GAMEENGINE_HPP_

#include "GameEngine/Layout/Widget.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/IGraphic.hpp"
#include <filesystem>
#include <memory>
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/WorldSerializer.hpp"
#include <vector>

namespace OrByte {

class GameEngine {
    public:
        enum GraphicalLibs : std::uint8_t {
            SFML,
        };

        GameEngine(GraphicalLibs graphicalLib = SFML);
        GameEngine(std::filesystem::path graphicPath);
        ~GameEngine() = default;

        void run();

    private:
        DynamicLibrary _graphicLoader;
        std::unique_ptr<IGraphic> _graphicLib;
        std::unique_ptr<Widget> _root;
        std::vector<std::unique_ptr<DynamicLibrary>> _componentLoaders;
        ComponentFactories _componentFactories;
        World _world;
        // std::vector<Event> _events;

        void _loadComponents();
        void _initLayout();

        void _handleEvents();
        bool _handleEvent(Event& event);
        void _update();
        void _render();
};

} // namespace OrByte

#endif /* GAMEENGINE_HPP_ */
