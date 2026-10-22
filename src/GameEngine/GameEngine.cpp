/*
** EPITECH PROJECT, 2026
** RType
** File description:
** GameEngine.cpp
*/

#include "GameEngine/GameEngine.hpp"
#include <memory>
#include <queue>
#include <string_view>

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

} // namespace

OrByte::GameEngine::GameEngine(GraphicalLibs graphicalLib) : _graphicLoader(getGraphicalPlugin(graphicalLib)), _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")())
{
    this->_graphicLib->open();
}

OrByte::GameEngine::GameEngine(std::filesystem::path graphicPath) : _graphicLoader(graphicPath), _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")())
{
    this->_graphicLib->open();
}

bool OrByte::GameEngine::_handleEvent(Event& event)
{
    if (this->_events.empty())
        return false;
    event = this->_events.back();
    this->_events.pop_back();

    // Handle Event

    return false;
}

void OrByte::GameEngine::_handleEvents()
{
    std::queue<Event> events = this->_graphicLib->listen();

    while (!events.empty()) {
        this->_events.push_back(events.front());
        events.pop();
    }

    for (auto event: this->_events) {
        if (event.type == Event::Type::Closed) {
            this->_graphicLib->close();
            return;
        }
        this->_handleEvent(event);
    }
}

void OrByte::GameEngine::_update()
{

}

void OrByte::GameEngine::_render()
{
    this->_graphicLib->clear();

    this->_graphicLib->update();
}

void OrByte::GameEngine::run()
{
    while (this->_graphicLib->isOpen()) {
        this->_handleEvents();
        if (!this->_graphicLib->isOpen())
            break;

        this->_update();
        this->_render();
    }
}
