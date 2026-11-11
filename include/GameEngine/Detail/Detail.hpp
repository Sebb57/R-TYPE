/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Detail.hpp
*/

#pragma once
#ifndef DETAIL_HPP_
    #define DETAIL_HPP_

    #include "GameEngine/ECS/WorldSerializer.hpp"
    #include "GameEngine/Layout/Widget.hpp"
    #include "plugins/IComponent.hpp"
    #include <cstddef>
    #include <cstdint>
    #include <functional>
    #include <set>
    #include <string>
    #include <vector>

namespace OrByte {

class Detail final : public Widget {
    public:
        struct Cell {
            std::string tag;
            std::function<std::string()> get;
            std::function<bool(const std::string&)> set;
            std::function<void()> click;
            bool acceptsImage = false;
        };

        struct Field {
            std::string label;
            std::vector<Cell> cells;
        };

        Detail(World& world, const ComponentFactories& factories, int width = 300);
        ~Detail() override = default;

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;
        [[nodiscard]] int getHeaderHeight() const override { return HEADER; }

        bool canAccept(const DragPayload& payload, Vector2 localMouse) override;
        void onDrop(const DragPayload& payload, Vector2 localMouse) override;

    private:
        static constexpr int HEADER = 24;
        static constexpr int SECTION_HEIGHT = 24;
        static constexpr int ROW_HEIGHT = 22;
        static constexpr int PADDING = 8;
        static constexpr int CELL_GAP = 4;

        enum class RowKind : std::uint8_t {
            Section,
            Value,
            AddButton,
            AddChoice,
        };

        struct Row {
            RowKind kind = RowKind::Value;
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
        const ComponentFactories& _factories;
        bool _addMenuOpen = false;
        float _scroll = 0.f;
        float _contentHeight = 0.f;
        int _shownEntity = -1;
        std::set<ComponentType> _collapsed;
        Edit _edit;

        [[nodiscard]] std::vector<Row> _buildRows(int entityId);
        [[nodiscard]] Rect _cellRect(const Row& row, std::size_t cellIndex, int width) const;
        [[nodiscard]] Rect _removeRect(const Row& row, int width) const;
        [[nodiscard]] std::vector<ComponentType> _addableComponents(int entityId) const;
        [[nodiscard]] bool _isEditing(const Row& row, std::size_t cellIndex) const;
        [[nodiscard]] bool _findImageCell(Vector2 localMouse, int& entityId, Row& found, std::size_t& cellIndex);

        void _syncSelection();
        void _clampScroll();
        void _startEdit(int entityId, const Row& row, std::size_t cellIndex);
        void _commitEdit();
        void _cancelEdit();
        void _editNextCell();
        void _addComponent(int entityId, ComponentType type);
        void _removeComponent(int entityId, ComponentType type);

        bool _onClick(Vector2 localMouse);
        bool _onText(const Event& event);
        bool _onKey(const Event& event);

        void _drawRow(const IGraphic& graphic, const Row& row, int width) const;
};

}

#endif /* DETAIL_HPP_ */
