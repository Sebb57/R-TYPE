/*
** EPITECH PROJECT, 2026
** RType
** File description:
** GameEngine.cpp
*/

#include "GameEngine/GameEngine.hpp"
#include "GameEngine/Layout/Panel.hpp"
#include "GameEngine/Layout/AssetsWidget.hpp"
#include "GameEngine/Layout/ConsoleWidget.hpp"
#include "GameEngine/Layout/TabWidget.hpp"
#include "GameEngine/Detail/Detail.hpp"
#include "GameEngine/Layout/EntityWidget.hpp"
#include "GameEngine/Layout/FilesManagementWidget.hpp"
#include "GameEngine/Layout/SceneManagerWidget.hpp"
#include "constants.hpp"
#include <exception>
#include "GameEngine/Layout/SceneView.hpp"
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

OrByte::GameEngine::GameEngine(GraphicalLibs graphicalLib)
    : GameEngine(getGraphicalPlugin(graphicalLib))
{
}

OrByte::GameEngine::GameEngine(std::filesystem::path graphicPath)
    : _graphicLoader(graphicPath),
      _graphicLib(this->_graphicLoader.loadFunction<IGraphic* (*)()>("factory")()),
      _components(loadComponentPlugins(constants::COMPONENTS_DIR)),
      _console(this->_scenes)
{
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
    } catch (const std::exception& error) {
        std::cerr << "[GameEngine] audio disabled: " << error.what() << std::endl;
        this->_audioSystem.reset();
        this->_audio.reset();
    }
}

void OrByte::GameEngine::_bindAudioToScene()
{
    // Release the previous scene's tracks before binding the active world.
    this->_audioSystem.reset();
    if (!this->_audio || !this->_audioSystemLoader || this->_scenes.empty())
        return;
    try {
        this->_audioSystem.reset(this->_audioSystemLoader->loadFunction<ISystem* (*)(World&, IAudio&)>("factory")(this->_scenes.currentWorld(), *this->_audio));
    } catch (const std::exception& error) {
        std::cerr << "[GameEngine] audio disabled: " << error.what() << std::endl;
    }
}

void OrByte::GameEngine::_initLayout()
{
    this->_bindAudioToScene();
    std::unique_ptr<Widget> scene;
    if (this->_scenes.empty())
        scene = std::make_unique<Panel>("", 0);
    else
        scene = std::make_unique<SceneView>(this->_scenes.currentWorld());

    Widget& topBar = scene->attach(Widget::Side::Top, std::make_unique<Panel>("", 104));
    Widget& toolbar = topBar.attach(Widget::Side::Top, std::make_unique<Panel>("", 40));
    toolbar.attach(Widget::Side::Left, std::make_unique<FilesManagementWidget>(this->_scenes, this->_components.factories, this->_console));
    topBar.attach(Widget::Side::Bottom, std::make_unique<SceneManagerWidget>(this->_scenes));

    auto bottom = std::make_unique<TabWidget>(220);
    TabWidget& tabs = *bottom;

    if (!this->_scenes.empty())
        tabs.addTab(std::make_unique<AssetsWidget>(this->_scenes.currentWorld()));
    const Widget& console = tabs.addTab(std::make_unique<ConsoleWidget>(this->_console));
    this->_console.setOnStart([&tabs, &console] { tabs.select(console); });
    scene->attach(Widget::Side::Bottom, std::move(bottom));
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
    this->_console.update();
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
