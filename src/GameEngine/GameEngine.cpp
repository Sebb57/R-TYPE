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
#include "constants.hpp"
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <queue>
#include <string_view>

namespace {

inline constexpr std::string_view PLUGINS_DIR = "./plugins";

#ifdef _WIN32
inline constexpr std::string_view PLUGIN_EXTENSION = ".dll";
#else
inline constexpr std::string_view PLUGIN_EXTENSION = ".so";
#endif

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

OrByte::GameEngine::GameEngine(GraphicalLibs graphicalLib) : _graphicLoader(getGraphicalPlugin(graphicalLib)), _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")()), _components(loadComponentPlugins(constants::COMPONENTS_DIR))
{
    this->_world.ensureCamera();
    this->_graphicLib->open();
    this->_loadAudio();
    this->_initLayout();
}

OrByte::GameEngine::GameEngine(std::filesystem::path graphicPath) : _graphicLoader(graphicPath), _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")()), _components(loadComponentPlugins(constants::COMPONENTS_DIR))
{
    this->_world.ensureCamera();
    this->_graphicLib->open();
    this->_loadAudio();
    this->_initLayout();
}

void OrByte::GameEngine::_loadAudio()
{
    const std::filesystem::path audioLib = std::filesystem::path(PLUGINS_DIR) / ("SFMLAudio" + std::string(PLUGIN_EXTENSION));
    const std::filesystem::path audioSystem = std::filesystem::path(PLUGINS_DIR) / "systems" / ("AudioSystem" + std::string(PLUGIN_EXTENSION));

    try {
        this->_audioLoader = std::make_unique<DynamicLibrary>(audioLib);
        this->_audio.reset(this->_audioLoader->loadFunction<IAudio* (*)()>("factory")());
        this->_audioSystemLoader = std::make_unique<DynamicLibrary>(audioSystem);
        this->_audioSystem.reset(this->_audioSystemLoader->loadFunction<ISystem* (*)(World&, IAudio&)>("factory")(this->_world, *this->_audio));
    } catch (const std::exception& error) {
        std::cerr << "[GameEngine] audio disabled: " << error.what() << std::endl;
        this->_audioSystem.reset();
        this->_audio.reset();
    }
}

void OrByte::GameEngine::_initLayout()
{
    auto scene = std::make_unique<SceneView>(this->_world);

    Widget& toolbar = scene->attach(Widget::Side::Top, std::make_unique<Panel>("", 40));
    toolbar.attach(Widget::Side::Left, std::make_unique<FilesManagementWidget>(this->_world, this->_components.factories));
    scene->attach(Widget::Side::Bottom, std::make_unique<Panel>("Console", 220));
    scene->attach(Widget::Side::Left, std::make_unique<EntityWidget>(this->_world, 250));
    scene->attach(Widget::Side::Right, std::make_unique<Detail>(this->_world, this->_components.factories, 300));

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
    if (this->_root)
        return this->_root->handleEvent(event);
    return false;
}

void OrByte::GameEngine::_update()
{
    if (this->_audioSystem)
        this->_audioSystem->update();
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
