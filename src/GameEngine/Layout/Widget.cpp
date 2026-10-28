/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Widget.cpp
*/

#include "GameEngine/Layout/Widget.hpp"
#include <algorithm>
#include <stdexcept>

namespace {

std::size_t sideIndex(OrByte::Widget::Side side)
{
    return static_cast<std::size_t>(side);
}

bool isMouseEvent(const OrByte::Event& event)
{
    return event.type == OrByte::Event::Type::MouseMoved
        || event.type == OrByte::Event::Type::MouseButtonPressed
        || event.type == OrByte::Event::Type::MouseButtonReleased
        || event.type == OrByte::Event::Type::MouseWheelScrolled;
}

Rect cutSide(Rect& area, OrByte::Widget::Side side, int size)
{
    const bool horizontal = side == OrByte::Widget::Side::Top || side == OrByte::Widget::Side::Bottom;
    const int thickness = std::clamp(size, 0, horizontal ? area.h : area.w);
    Rect band;

    switch (side) {
        case OrByte::Widget::Side::Top:
            band = Rect(area.x, area.y, area.w, thickness);
            area.y += thickness;
            area.h -= thickness;
            break;
        case OrByte::Widget::Side::Right:
            band = Rect(area.x + area.w - thickness, area.y, thickness, area.h);
            area.w -= thickness;
            break;
        case OrByte::Widget::Side::Bottom:
            band = Rect(area.x, area.y + area.h - thickness, area.w, thickness);
            area.h -= thickness;
            break;
        case OrByte::Widget::Side::Left:
            band = Rect(area.x, area.y, thickness, area.h);
            area.x += thickness;
            area.w -= thickness;
            break;
    }
    return band;
}

} // namespace

OrByte::Widget::Widget(std::string title, int size) : _title(std::move(title)), _size(std::max(size, 0))
{
}

OrByte::Widget& OrByte::Widget::attach(Side side, std::unique_ptr<Widget> child)
{
    if (!child)
        throw std::invalid_argument("Widget::attach: null child");

    std::unique_ptr<Widget>& slot = this->_children.at(sideIndex(side));

    slot = std::move(child);
    slot->_parent = this;
    this->_layout();
    return *slot;
}

std::unique_ptr<OrByte::Widget> OrByte::Widget::detach(Side side)
{
    std::unique_ptr<Widget> child = std::move(this->_children.at(sideIndex(side)));

    if (child)
        child->_parent = nullptr;
    this->_layout();
    return child;
}

OrByte::Widget *OrByte::Widget::getChild(Side side) const noexcept
{
    return this->_children.at(sideIndex(side)).get();
}

void OrByte::Widget::setRect(const Rect& rect)
{
    this->_rect = Rect(rect.x, rect.y, std::max(rect.w, 0), std::max(rect.h, 0));
    this->_layout();
}

void OrByte::Widget::setSize(int size)
{
    this->_size = std::max(size, 0);
    this->_layoutParent();
}

void OrByte::Widget::setVisible(bool visible)
{
    this->_visible = visible;
    this->_layoutParent();
}

void OrByte::Widget::_layout()
{
    Rect area = this->_rect;

    for (std::size_t i = 0; i < SIDE_COUNT; i++) {
        Widget *child = this->_children.at(i).get();

        if (child == nullptr || !child->_visible)
            continue;
        child->setRect(cutSide(area, static_cast<Side>(i), child->_size));
    }
    this->_content = area;
}

void OrByte::Widget::_layoutParent()
{
    if (this->_parent != nullptr)
        this->_parent->_layout();
}

void OrByte::Widget::render(const IGraphic& graphic)
{
    if (!this->_visible)
        return;

    if (this->_content.w > 0 && this->_content.h > 0) {
        graphic.pushRegion(this->_content);
        this->draw(graphic, Vector2u(static_cast<unsigned>(this->_content.w), static_cast<unsigned>(this->_content.h)));
        graphic.popRegion();
    }
    for (const std::unique_ptr<Widget>& child : this->_children) {
        if (child)
            child->render(graphic);
    }
}

bool OrByte::Widget::handleEvent(const Event& event)
{
    if (event.type == Event::Type::Resized && this->_parent == nullptr) {
        this->setRect(Rect(0, 0, event.width, event.height));
        return false;
    }
    if (isMouseEvent(event))
        return this->_dispatchMouse(event);
    return this->_dispatch(event);
}

bool OrByte::Widget::_dispatchMouse(const Event& event)
{
    if (!this->_visible || !this->_rect.contains(event.windowX, event.windowY))
        return false;

    for (const std::unique_ptr<Widget>& child : this->_children) {
        if (child && child->_dispatchMouse(event))
            return true;
    }
    if (!this->_content.contains(event.windowX, event.windowY))
        return false;
    return this->onEvent(event, Vector2(static_cast<float>(event.windowX - this->_content.x),
        static_cast<float>(event.windowY - this->_content.y)));
}

bool OrByte::Widget::_dispatch(const Event& event)
{
    if (!this->_visible)
        return false;

    for (const std::unique_ptr<Widget>& child : this->_children) {
        if (child && child->_dispatch(event))
            return true;
    }
    return this->onEvent(event, Vector2(static_cast<float>(event.windowX - this->_content.x),
        static_cast<float>(event.windowY - this->_content.y)));
}
