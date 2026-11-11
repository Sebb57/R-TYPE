/*
** EPITECH PROJECT, 2026
** RType
** File description:
** TabWidget.hpp
*/

#pragma once
#ifndef TABWIDGET_HPP_
    #define TABWIDGET_HPP_

    #include "Widget.hpp"
    #include <cstddef>
    #include <memory>
    #include <string>
    #include <vector>

namespace OrByte {

class TabWidget final : public Widget {
    public:
        explicit TabWidget(int size = 0);
        ~TabWidget() override = default;

        TabWidget(const TabWidget&) = delete;
        TabWidget& operator=(const TabWidget&) = delete;
        TabWidget(TabWidget&&) = delete;
        TabWidget& operator=(TabWidget&&) = delete;

        Widget& addTab(std::unique_ptr<Widget> page);
        void select(std::size_t index);
        void select(const Widget& page);
        [[nodiscard]] std::size_t getSelected() const noexcept { return this->_selected; }

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;
        [[nodiscard]] int getHeaderHeight() const override { return TAB_HEIGHT; }

    private:
        struct Tab {
            std::string label;
            Widget *page = nullptr;
            std::unique_ptr<Widget> stored;
            float x = 0.f;
            float width = 0.f;
        };

        static constexpr int TAB_HEIGHT = 26;

        std::vector<Tab> _tabs;
        std::size_t _selected = 0;
};

} // namespace OrByte

#endif /* TABWIDGET_HPP_ */
