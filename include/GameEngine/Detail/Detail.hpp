/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Detail.hpp
*/

#pragma once
#ifndef DETAIL_HPP_
    #define DETAIL_HPP_

    #include "GameEngine/Layout/Widget.hpp"
    #include "plugins/IComponent.hpp"
    #include <cstddef>
    #include <functional>
    #include <set>
    #include <string>
    #include <vector>

class World;

namespace OrByte {

class Detail final : public Widget {
    public:
        struct Cell {
            std::string tag;
            std::function<std::string()> get;
            std::function<bool(const std::string&)> set;
            std::function<void()> click;
        };

        struct Field {
            std::string label;
            std::vector<Cell> cells;
        };

        explicit Detail(World& world, int width = 300);
        ~Detail() override = default;

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;
        [[nodiscard]] int getHeaderHeight() const override { return HEADER; }

    private:
        static constexpr int HEADER = 24;
        static constexpr int SECTION_HEIGHT = 24;
        static constexpr int ROW_HEIGHT = 22;
        static constexpr int PADDING = 8;
        static constexpr int CELL_GAP = 4;

        struct Row {
            float y = 0.f;
            float height = 0.f;
            ComponentType type = ComponentType::ENUM_END;
            int fieldIndex = -1;
            Field field;
        };

        struct Edit {
            bool active = false;
            int entity = -1;
            ComponentType type = ComponentType::ENUM_END;
            int fieldIndex = -1;
            int cellIndex = -1;
            std::string text;
            bool selectAll = true;
        };

        World& _world;
        float _scroll = 0.f;
        float _contentHeight = 0.f;
        int _shownEntity = -1;
        std::set<ComponentType> _collapsed;
        Edit _edit;

        [[nodiscard]] std::vector<Row> _buildRows(int entityId);
        [[nodiscard]] Rect _cellRect(const Row& row, std::size_t cellIndex, int width) const;
        [[nodiscard]] bool _isEditing(const Row& row, std::size_t cellIndex) const;

        void _syncSelection();
        void _clampScroll();
        void _startEdit(int entityId, const Row& row, std::size_t cellIndex);
        void _commitEdit();
        void _cancelEdit();
        void _editNextCell();

        bool _onClick(Vector2 localMouse);
        bool _onText(const Event& event);
        bool _onKey(const Event& event);

        void _drawRow(const IGraphic& graphic, const Row& row, int width) const;
};

}

#endif /* DETAIL_HPP_ */
