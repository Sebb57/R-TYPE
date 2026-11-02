/*
** EPITECH PROJECT, 2026
** RType
** File description:
** EntityWidget.hpp
*/

#pragma once
#ifndef ENTITYWIDGET_HPP_
    #define ENTITYWIDGET_HPP_

    #include "GameEngine/ECS/World.hpp"
    #include "GameEngine/Layout/Panel.hpp"
    #include <string>
    #include <vector>

namespace OrByte {

class EntityWidget final : public Panel {
    public:
        explicit EntityWidget(World& world, int width = 480);
        ~EntityWidget() override = default;

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        World& _world;
        std::size_t _selectedIndex = 0;
        std::size_t _scrollOffset = 0;
        bool _editing = false;
        int _editingEntityId = -1;
        std::string _editingText;

        [[nodiscard]] std::vector<int> _entityIds() const;
        [[nodiscard]] std::string _nameOf(int entityId) const;
        void _startEditing(int entityId);
        void _finishEditing();
        void _cancelEditing();
        void _applyTextInput(const Event& event);
        void _applyKeyAction(const Event& event);
        void _selectRow(Vector2 localMouse);
        void _setSelectedIndex(std::size_t index);
};

} // namespace OrByte

#endif /* ENTITYWIDGET_HPP_ */
