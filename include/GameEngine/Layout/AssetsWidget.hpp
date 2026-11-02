/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AssetsWidget.hpp
*/

#pragma once
#ifndef ASSETSWIDGET_HPP_
    #define ASSETSWIDGET_HPP_

#include "GameEngine/ECS/World.hpp"
#include "Widget.hpp"
#include <dirent.h>
#include <filesystem>

namespace OrByte {

class AssetsWidget final : public Widget {
    public:
        AssetsWidget() = delete;
        AssetsWidget(World& world, int width = 480);
        ~AssetsWidget() override = default;

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        enum SortType {
            Name,
            Size
        };

        World& _world;

        std::filesystem::path _dirPath;
        DIR* _dir;
        std::vector<std::string> _entries;

        bool _showHidden = false;
        SortType _sortType = SortType::Name;
        bool _ascending = true;
        float _scrollOffset = 0.f;

        std::size_t _hovered = -1;
        std::size_t _selected = -1;

        void _getEntries();
        void _sortEntries(SortType type, bool ascending = true);
};

} // namespace OrByte

#endif /* ASSETSWIDGET_HPP_ */
