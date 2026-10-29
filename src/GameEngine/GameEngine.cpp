/*
** EPITECH PROJECT, 2026
** RType
** File description:
** GameEngine.cpp
*/

#include "GameEngine/GameEngine.hpp"
#include "GameEngine/Layout/Panel.hpp"
#include "GameEngine/Layout/SceneView.hpp"
#include "GameEngine/Layout/EntityWidget.hpp"
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
    for (int i = 0; i < 50; ++i) {
        this->_world.createEntity();
    }
    this->_graphicLib->open();
    this->_initLayout();
}

OrByte::GameEngine::GameEngine(std::filesystem::path graphicPath) : _graphicLoader(graphicPath), _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")())
{
    this->_graphicLib->open();
    this->_initLayout();
}

void OrByte::GameEngine::_initLayout()
{
    auto scene = std::make_unique<SceneView>(
        [this](const IGraphic& graphicLib) {
            (void) graphicLib; // TODO: remove when below is done and using this parameter
            // TODO: Game Render
        });

    scene->attach(Widget::Side::Top, std::make_unique<Panel>("", 40)); // ToolBar
    scene->attach(Widget::Side::Bottom, std::make_unique<Panel>("Console", 220));
    scene->attach(Widget::Side::Left, std::make_unique<EntityWidget>(this->_world, 250));
    scene->attach(Widget::Side::Right, std::make_unique<Panel>("Inspector", 300));

    const auto [w, h] = this->_graphicLib->getWindowSize();

    scene->setRect(Rect(0, 0, static_cast<int>(w), static_cast<int>(h)));
    this->_root = std::move(scene);
}

void OrByte::GameEngine::_handleEvents()
{
    std::queue<Event> events = this->_graphicLib->listen();

    while (!events.empty()) {
        Event event = events.front();
        events.pop();

        if (event.type == Event::Type::Closed) {
            this->_graphicLib->close();
            return;
        }
        this->_handleEvent(event);
    }
}

bool OrByte::GameEngine::_handleEvent(Event& event)
{
    if (this->_root)
        return this->_root->handleEvent(event);
    return false;
}

void OrByte::GameEngine::_update()
{

}

void OrByte::GameEngine::_render()
{
    this->_graphicLib->clear();

    if (this->_root)
        this->_root->render(*this->_graphicLib);

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
