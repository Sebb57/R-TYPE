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
        void setResizable(bool resizable) noexcept { this->_resizable = resizable; }
        [[nodiscard]] bool isResizable() const noexcept { return this->_resizable; }
        void setMinSize(int minSize);
        [[nodiscard]] int getMinSize() const noexcept { return this->_minSize; }
        [[nodiscard]] Side getSide() const noexcept { return this->_side; }
        [[nodiscard]] CursorType getCursor() const noexcept { return this->_cursor; }

        void render(const IGraphic& graphic);
        bool handleEvent(const Event& event);

    protected:
        virtual void draw([[maybe_unused]] const IGraphic& graphic, [[maybe_unused]] Vector2u size) {}
        virtual bool onEvent([[maybe_unused]] const Event& event, [[maybe_unused]] Vector2 localMouse) { return false; }

    private:
        struct Drag {
            Widget *target = nullptr;
            int startSize = 0;
            int startPos = 0;
        };

        static constexpr int GRIP = 4;
        static constexpr int MIN_CONTENT = 50;

        std::string _title;
        int _size = 0;
        bool _visible = true;
        Rect _rect;
        Rect _content;
        Widget *_parent = nullptr;
        std::array<std::unique_ptr<Widget>, SIDE_COUNT> _children;

        Side _side = Side::Top;
        bool _resizable = true;
        int _minSize = 40;
        Drag _drag;
        CursorType _cursor = CursorType::Arrow;

        void _layout();
        void _layoutParent();
        bool _dispatchMouse(const Event& event);
        bool _dispatch(const Event& event);
        
        [[nodiscard]] bool _gripContains(int x, int y) const;
        Widget *_findGrip(int x, int y);
        [[nodiscard]] int _maxSize() const;
        bool _handleResize(const Event& event);
};

} // namespace OrByte

#endif /* WIDGET_HPP_ */
