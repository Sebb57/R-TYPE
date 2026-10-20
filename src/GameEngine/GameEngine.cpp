/*
** EPITECH PROJECT, 2026
** RType
** File description:
** GameEngine.cpp
*/

#include "GameEngine/GameEngine.hpp"
#include <memory>
#include <string_view>
#include <unordered_map>

namespace {

inline constexpr std::string_view PLUGINS_DIR = "./plugins";

std::filesystem::path getGraphicalPlugin(OrByte::GameEngine::GraphicalLibs graphicalLib)
{
    std::filesystem::path path;

    switch (graphicalLib) {
        case OrByte::GameEngine::GraphicalLibs::SFML:
            path = "SFML.so";
            break;
        default:
            path = "SFML.so";
            break;
    }

    return PLUGINS_DIR / path;
}

}

OrByte::GameEngine::GameEngine(GraphicalLibs graphicalLib) : _graphicLoader(getGraphicalPlugin(graphicalLib)), _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")())
{}

OrByte::GameEngine::GameEngine(std::filesystem::path graphicPath) : _graphicLoader(graphicPath), _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")())
{}
