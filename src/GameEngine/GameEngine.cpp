/*
** EPITECH PROJECT, 2026
** RType
** File description:
** GameEngine.cpp
*/

#include "GameEngine/GameEngine.hpp"
#include "GameEngine/Layout/Panel.hpp"
#include "GameEngine/Detail/Detail.hpp"
#include "GameEngine/Layout/SceneView.hpp"
#include "GameEngine/Layout/EntityWidget.hpp"
#include "GameEngine/Layout/FilesManagementWidget.hpp"
#include "GameEngine/Layout/SceneManagerWidget.hpp"
#include "constants.hpp"
#include <filesystem>
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

OrByte::GameEngine::GameEngine(GraphicalLibs graphicalLib)
    : GameEngine(getGraphicalPlugin(graphicalLib))
{
}

OrByte::GameEngine::GameEngine(std::filesystem::path graphicPath)
    : _graphicLoader(graphicPath),
      _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")()),
      _components(loadComponentPlugins(constants::COMPONENTS_DIR))
{
    this->_graphicLib->open();
    this->_initLayout();
}

void OrByte::GameEngine::_initLayout()
{
    std::unique_ptr<Widget> scene;
    if (this->_scenes.empty())
        scene = std::make_unique<Panel>("", 0);
    else
        scene = std::make_unique<SceneView>(this->_scenes.currentWorld());

    Widget& topBar = scene->attach(Widget::Side::Top, std::make_unique<Panel>("", 104));
    Widget& toolbar = topBar.attach(Widget::Side::Top, std::make_unique<Panel>("", 40));
    toolbar.attach(Widget::Side::Left, std::make_unique<FilesManagementWidget>(this->_scenes, this->_components.factories));
    topBar.attach(Widget::Side::Bottom, std::make_unique<SceneManagerWidget>(this->_scenes));
    scene->attach(Widget::Side::Bottom, std::make_unique<Panel>("Console", 220));
    if (!this->_scenes.empty()) {
        World& world = this->_scenes.currentWorld();
        scene->attach(Widget::Side::Left, std::make_unique<EntityWidget>(world, 250));
        scene->attach(Widget::Side::Right, std::make_unique<Detail>(world, this->_components.factories, 300));
    }

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
    if (this->_root)
        this->_graphicLib->setCursor(this->_root->getCursor());
}

bool OrByte::GameEngine::_handleEvent(Event& event)
{
    if (!this->_root)
        return false;
    const auto revision = this->_scenes.revision();
    const bool handled = this->_root->handleEvent(event);
    // Rebuild after dispatch so the widget handling the event stays alive.
    if (revision != this->_scenes.revision()) {
        const Rect window = this->_root->getRect();
        this->_initLayout();
        this->_root->setRect(window);
    }
    return handled;
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
