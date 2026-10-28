/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Widget.hpp
*/

#pragma once
#ifndef WIDGET_HPP_
    #define WIDGET_HPP_

    #include "GameEngine/ECS/Utils.hpp"
    #include "GameEngine/Event.hpp"
    #include "plugins/IGraphic.hpp"
    #include <array>
    #include <cstddef>
    #include <cstdint>
    #include <memory>
    #include <string>
    #include <utility>

namespace OrByte {

class Widget {
    public:
        enum class Side : std::uint8_t {
            Top,
            Right,
            Bottom,
            Left,
        };

        static constexpr std::size_t SIDE_COUNT = 4;

        explicit Widget(std::string title = "", int size = 0);
        virtual ~Widget() = default;

        Widget& operator=(const Widget& object) = delete;
        Widget& operator=(Widget&& object) = delete;
        Widget(const Widget& object) = delete;
        Widget(Widget&& object) = delete;

        Widget& attach(Side side, std::unique_ptr<Widget> child);
        std::unique_ptr<Widget> detach(Side side);
        [[nodiscard]] Widget *getChild(Side side) const noexcept;
        [[nodiscard]] Widget *getParent() const noexcept { return this->_parent; }

        void setRect(const Rect& rect);
        [[nodiscard]] const Rect& getRect() const noexcept { return this->_rect; }
        [[nodiscard]] const Rect& getContentRect() const noexcept { return this->_content; }

        void setSize(int size);
        [[nodiscard]] int getSize() const noexcept { return this->_size; }
        void setVisible(bool visible);
        [[nodiscard]] bool isVisible() const noexcept { return this->_visible; }
        void setTitle(std::string title) { this->_title = std::move(title); }
        [[nodiscard]] const std::string& getTitle() const noexcept { return this->_title; }

        void render(const IGraphic& graphic);
        bool handleEvent(const Event& event);

    protected:
        virtual void draw([[maybe_unused]] const IGraphic& graphic, [[maybe_unused]] Vector2u size) {}
        virtual bool onEvent([[maybe_unused]] const Event& event, [[maybe_unused]] Vector2 localMouse) { return false; }

    private:
        std::string _title;
        int _size = 0;
        bool _visible = true;
        Rect _rect;
        Rect _content;
        Widget *_parent = nullptr;
        std::array<std::unique_ptr<Widget>, SIDE_COUNT> _children;

        void _layout();
        void _layoutParent();
        bool _dispatchMouse(const Event& event);
        bool _dispatch(const Event& event);
};

} // namespace OrByte

#endif /* WIDGET_HPP_ */
