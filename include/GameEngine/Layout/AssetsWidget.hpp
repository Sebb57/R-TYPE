/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AssetsWidget.hpp
*/

#pragma once
#ifndef ASSETSWIDGET_HPP_
    #define ASSETSWIDGET_HPP_

#include "Chronometer.hpp"
#include "GameEngine/ECS/World.hpp"
#include "Widget.hpp"
#include <dirent.h>
#include <filesystem>
#include <vector>

namespace OrByte {

class AssetsWidget final : public Widget {
    public:
        AssetsWidget() = delete;
        AssetsWidget(World& world, int width = 480);
        ~AssetsWidget() override = default;
        void setDirPath(const std::filesystem::path& path);
        [[nodiscard]] const std::filesystem::path& getDirPath() const { return this->_dirPath; }

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        enum SortType {
            Name,
            Size
        };

        struct DirectoryButton {
            float x;
            float y;
            float w;
            float h;
            std::filesystem::path path;
        };

        World& _world;

        std::filesystem::path _startPath;
        std::filesystem::path _dirPath;
        std::vector<std::string> _entries;

        bool _showHidden = false;
        SortType _sortType = SortType::Name;
        bool _ascending = true;
        float _scrollOffset = 0.f;

        std::size_t _hovered = -1;
        std::size_t _selected = -1;
        Chronometer _chronoClick;
        std::vector<DirectoryButton> _dirButtons;

        std::size_t _dragCandidate = static_cast<std::size_t>(-1);
        Vector2 _pressPos = { 0.f, 0.f };

        void _getEntries();
        void _sortEntries(SortType type, bool ascending = true);
        void _navigateTo(const std::filesystem::path& path);
        [[nodiscard]] float _titleHeight() const;
        [[nodiscard]] float _headerHeight() const;
};

} // namespace OrByte

#endif /* ASSETSWIDGET_HPP_ */
