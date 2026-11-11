/**
 * @file FilesManagementWidget.hpp
 * @brief toolbar buttons to compile, save and load the world
 *
 * @author Seb
 * @date 2026-10-29
**/

#ifndef FILESMANAGEMENTWIDGET_HPP_
#define FILESMANAGEMENTWIDGET_HPP_

#include "GameEngine/Console/Console.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/WorldSerializer.hpp"
#include "GameEngine/ECS/SceneCollection.hpp"
#include "Widget.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace OrByte {

class FilesManagementWidget final : public Widget {
    public:
        FilesManagementWidget(SceneCollection& scenes, const ComponentFactories& factories, Console& console, int width = 480);
        ~FilesManagementWidget() override = default;

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        enum class Mode : std::uint8_t {
            Idle,
            Picking,
            Naming,
        };

        SceneCollection& _scenes;
        const ComponentFactories& _factories;
        Console& _console;
        int _baseWidth;
        Mode _mode = Mode::Idle;
        std::vector<std::filesystem::path> _files;
        std::size_t _scrollOffset = 0;
        std::string _fileName;
        std::string _defaultName;

        void _compile();
        void _save();
        void _load(const std::filesystem::path& path);
        void _openPicker();
        void _openNaming();
        void _close();
        void _refreshFiles();
        void _resize(int needed);
        void _onClick(Vector2 localMouse);
        bool _onNamingEvent(const Event& event);
        void _drawPicker(const IGraphic& graphic, int width, int height) const;
        void _drawNaming(const IGraphic& graphic, int height) const;
};

} // namespace OrByte

#endif /* FILESMANAGEMENTWIDGET_HPP_ */
