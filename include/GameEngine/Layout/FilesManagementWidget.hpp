/**
 * @file FilesManagementWidget.hpp
 * @brief toolbar buttons to compile, save and load the world
 *
 * @author Seb
 * @date 2026-10-29
**/

#ifndef FILESMANAGEMENTWIDGET_HPP_
#define FILESMANAGEMENTWIDGET_HPP_

#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/WorldSerializer.hpp"
#include "Widget.hpp"
#include <cstddef>
#include <filesystem>
#include <vector>

namespace OrByte {

class FilesManagementWidget final : public Widget {
    public:
        FilesManagementWidget(World& world, const ComponentFactories& factories, int width = 480);
        ~FilesManagementWidget() override = default;

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        World& _world;
        const ComponentFactories& _factories;
        int _baseWidth;
        bool _picking = false;
        std::vector<std::filesystem::path> _files;
        std::size_t _scrollOffset = 0;

        void _compile();
        void _save();
        void _load(const std::filesystem::path& path);
        void _openPicker();
        void _closePicker();
        void _refreshFiles();
        void _onClick(Vector2 localMouse);
};

} // namespace OrByte

#endif /* FILESMANAGEMENTWIDGET_HPP_ */
