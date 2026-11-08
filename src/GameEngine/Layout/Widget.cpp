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

constexpr float PREVIEW_SIZE = 64.f;
constexpr float PREVIEW_OPACITY = 0.55f;
constexpr float FONT_SIZE = 12.f;

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

bool resizesVertically(OrByte::Widget::Side side)
{
    return side == OrByte::Widget::Side::Top || side == OrByte::Widget::Side::Bottom;
}

OrByte::Widget::Side oppositeSide(OrByte::Widget::Side side)
{
    return static_cast<OrByte::Widget::Side>((sideIndex(side) + 2) % OrByte::Widget::SIDE_COUNT);
}

OrByte::CursorType cursorFor(OrByte::Widget::Side side)
{
    return resizesVertically(side) ? OrByte::CursorType::ResizeVertical : OrByte::CursorType::ResizeHorizontal;
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
    slot->_side = side;
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

void OrByte::Widget::setMinSize(int minSize)
{
    this->_minSize = std::max(minSize, 0);
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
    for (const std::unique_ptr<Widget>& child : this->_children)
        if (child)
            child->render(graphic);

    if (this->_parent == nullptr) {
        this->_renderDropPreview(graphic);
        this->_renderPayload(graphic);
    }
    if (this->_parent == nullptr)
        this->_renderDropPreview(graphic);
}

bool OrByte::Widget::handleEvent(const Event& event)
{
    if (event.type == Event::Type::Resized && this->_parent == nullptr) {
        this->setRect(Rect(0, 0, event.width, event.height));
        return false;
    }
    if (this->_parent == nullptr && this->_payload.active && !isMouseEvent(event)) {
        if (event.type == Event::Type::KeyPressed && event.key == Event::Key::Escape)
            this->_payload = Payload{};
        return true;
    }
    if (isMouseEvent(event)) {
        if (this->_parent == nullptr) {
            this->_mouseX = event.windowX;
            this->_mouseY = event.windowY;
            if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Left)
                this->_leftDown = true;

            if (this->_payload.active) {
                const bool consumed = this->_handlePayload(event);

                if (event.type == Event::Type::MouseButtonReleased && event.mouseButton == Event::MouseButton::Left)
                    this->_leftDown = false;
                return consumed;
            }
            if (this->_move.active)
                return this->_handleMove(event);
            if (this->_handleResize(event) || this->_handleMove(event))
                return true;
        }
        const bool consumed = this->_dispatchMouse(event);

        if (this->_parent == nullptr && event.type == Event::Type::MouseButtonReleased
            && event.mouseButton == Event::MouseButton::Left)
            this->_leftDown = false;
        return consumed;
    }
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
    return this->onEvent(event, Vector2(static_cast<float>(event.windowX - this->_content.x), static_cast<float>(event.windowY - this->_content.y)));
}

bool OrByte::Widget::_dispatch(const Event& event)
{
    if (!this->_visible)
        return false;

    for (const std::unique_ptr<Widget>& child : this->_children) {
        if (child && child->_dispatch(event))
            return true;
    }
    return this->onEvent(event, Vector2(static_cast<float>(event.windowX - this->_content.x), static_cast<float>(event.windowY - this->_content.y)));
}

bool OrByte::Widget::_gripContains(int x, int y) const
{
    if (this->_parent == nullptr || !this->_resizable || !this->_visible)
        return false;

    const bool inX = x >= this->_rect.x && x < this->_rect.x + this->_rect.w;
    const bool inY = y >= this->_rect.y && y < this->_rect.y + this->_rect.h;

    switch (this->_side) {
        case Side::Top:
            return inX && y >= this->_rect.y + this->_rect.h - GRIP && y < this->_rect.y + this->_rect.h + GRIP;
        case Side::Bottom:
            return inX && y >= this->_rect.y - GRIP && y < this->_rect.y + GRIP;
        case Side::Left:
            return inY && x >= this->_rect.x + this->_rect.w - GRIP && x < this->_rect.x + this->_rect.w + GRIP;
        case Side::Right:
            return inY && x >= this->_rect.x - GRIP && x < this->_rect.x + GRIP;
    }
    return false;
}

OrByte::Widget *OrByte::Widget::_findGrip(int x, int y)
{
    for (const std::unique_ptr<Widget>& child : this->_children) {
        if (!child || !child->_visible)
            continue;
        if (Widget *nested = child->_findGrip(x, y))
            return nested;
        if (child->_gripContains(x, y))
            return child.get();
    }
    return nullptr;
}

int OrByte::Widget::_maxSize() const
{
    const Widget *opposite = this->_parent->_children.at(sideIndex(oppositeSide(this->_side))).get();
    const int oppositeSize = (opposite != nullptr && opposite->_visible) ? opposite->_size : 0;
    const int extent = resizesVertically(this->_side) ? this->_parent->_rect.h : this->_parent->_rect.w;

    return std::max(extent - oppositeSize - MIN_CONTENT, this->_effectiveMinSize());
}

bool OrByte::Widget::_handleResize(const Event& event)
{
    if (this->_drag.target != nullptr) {
        Widget *target = this->_drag.target;
        const bool vertical = resizesVertically(target->_side);
        const int pos = vertical ? event.windowY : event.windowX;

        this->_cursor = cursorFor(target->_side);
        if (event.type == Event::Type::MouseMoved) {
            int delta = pos - this->_drag.startPos;

            if (target->_side == Side::Bottom || target->_side == Side::Right)
                delta = -delta;
            target->setSize(std::clamp(this->_drag.startSize + delta, target->_effectiveMinSize(), target->_maxSize()));
        } else if (event.type == Event::Type::MouseButtonReleased && event.mouseButton == Event::MouseButton::Left) {
            this->_drag = Drag{};
        }
        return true;
    }

    if (event.type == Event::Type::MouseMoved) {
        const Widget *hovered = this->_findGrip(event.windowX, event.windowY);

        this->_cursor = hovered != nullptr ? cursorFor(hovered->_side) : CursorType::Arrow;
    }

    if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Left) {
        Widget *grip = this->_findGrip(event.windowX, event.windowY);

        if (grip != nullptr) {
            const bool vertical = resizesVertically(grip->_side);

            this->_drag = Drag{ grip, vertical ? grip->_rect.h : grip->_rect.w, vertical ? event.windowY : event.windowX };
            return true;
        }
    }
    return false;
}

bool OrByte::Widget::_isInside(const Widget *ancestor) const
{
    for (const Widget *widget = this; widget; widget = widget->_parent)
        if (widget == ancestor)
            return true;
    return false;
}

OrByte::Widget *OrByte::Widget::_findHeader(int x, int y)
{
    if (!this->_visible)
        return nullptr;

    for (const std::unique_ptr<Widget>& child : this->_children)
        if (child)
            if (Widget *found = child->_findHeader(x, y))
                return found;

    const int header = this->getHeaderHeight();

    if (this->_parent == nullptr || !this->_movable || header <= 0)
        return nullptr;

    const Rect bar(this->_content.x, this->_content.y, this->_content.w, std::min(header, this->_content.h));

    return bar.contains(x, y) ? this : nullptr;
}

OrByte::Widget *OrByte::Widget::_findDropTarget(int x, int y, const Widget *source)
{
    if (!this->_visible || this == source || !this->_rect.contains(x, y))
        return nullptr;

    for (const std::unique_ptr<Widget>& child : this->_children)
        if (child)
            if (Widget *found = child->_findDropTarget(x, y, source))
                return found;

    return this;
}

int OrByte::Widget::_dockSize(const Widget& target, Side side)
{
    const int extent = resizesVertically(side) ? target._content.h : target._content.w;

    return extent / (target._parent == nullptr ? 3 : 2);
}

bool OrByte::Widget::_canDock(const Widget& source, const Widget& target, Side side)
{
    if (target._isInside(&source))
        return false;

    const Widget *occupant = target._children.at(sideIndex(side)).get();

    if (occupant == &source)
        return false;
    if (occupant != nullptr && source._children.at(sideIndex(side)) != nullptr)
        return false;

    const int extent = resizesVertically(side) ? target._content.h : target._content.w;
    const int size = _dockSize(target, side);
    const int keep = target._parent == nullptr ? MIN_CONTENT : target._minSize;

    return size >= source._minSize && extent - size >= keep;
}

bool OrByte::Widget::_pickDropSide(const Widget& source, int x, int y, Side& side) const
{
    const Rect& c = this->_content;

    if (c.w <= 0 || c.h <= 0)
        return false;

    std::array<std::pair<float, Side>, SIDE_COUNT> candidates = {{
        { static_cast<float>(y - c.y) / static_cast<float>(c.h), Side::Top },
        { static_cast<float>(c.x + c.w - x) / static_cast<float>(c.w), Side::Right },
        { static_cast<float>(c.y + c.h - y) / static_cast<float>(c.h), Side::Bottom },
        { static_cast<float>(x - c.x) / static_cast<float>(c.w), Side::Left },
    }};

    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (const auto& [distance, candidate] : candidates)
        if (this->_canDock(source, *this, candidate)) {
            side = candidate;
            return true;
        }

    return false;
}

void OrByte::Widget::_dock(Widget& source, Widget& target, Side side)
{
    if (source._parent == nullptr || !OrByte::Widget::_canDock(source, target, side)) // ouin ouin jpp mettre de this dans les static
        return;

    Widget *parent = source._parent;
    const Side from = source._side;

    std::unique_ptr<Widget> moved = parent->detach(from);

    if (std::unique_ptr<Widget> outer = moved->detach(from))
        parent->attach(from, std::move(outer));

    const int size = OrByte::Widget::_dockSize(target, side);
    const Widget *occupant = target._children.at(sideIndex(side)).get();
    const int pushed = (occupant != nullptr && occupant->_visible) ? occupant->_size : 0;

    std::unique_ptr<Widget> displaced = target.detach(side);

    if (displaced)
        moved->attach(side, std::move(displaced));
    moved->_size = size + pushed;
    target.attach(side, std::move(moved));
}

void OrByte::Widget::_updateDropTarget(int x, int y)
{
    Move& move = this->_move;
    Widget *target = this->_findDropTarget(x, y, move.source);
    Side side {};

    move.dropTarget = nullptr;
    if (target == nullptr || !target->_content.contains(x, y))
        return;
    if (target->_pickDropSide(*move.source, x, y, side)) {
        move.dropTarget = target;
        move.dropSide = side;
    }
}

bool OrByte::Widget::_handleMove(const Event& event)
{
    Move& move = this->_move;

    if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Left) {
        Widget *header = this->_findHeader(event.windowX, event.windowY);

        move = Move{};
        if (header != nullptr) {
            move.source = header;
            move.pressed = true;
            move.startX = event.windowX;
            move.startY = event.windowY;
        }
        return false;
    }
    if (!move.pressed)
        return false;

    if (event.type == Event::Type::MouseMoved) {
        if (!move.active) {
            const int dx = event.windowX - move.startX;
            const int dy = event.windowY - move.startY;

            if (dx * dx + dy * dy < MOVE_THRESHOLD * MOVE_THRESHOLD)
                return false;
            move.active = true;
        }
        this->_cursor = CursorType::Move;
        this->_updateDropTarget(event.windowX, event.windowY);
        return true;
    }
    if (event.type == Event::Type::MouseButtonReleased && event.mouseButton == Event::MouseButton::Left) {
        const bool wasActive = move.active;

        if (wasActive && move.dropTarget != nullptr)
            _dock(*move.source, *move.dropTarget, move.dropSide);
        move = Move{};
        if (wasActive)
            this->_cursor = CursorType::Arrow;
        return wasActive;
    }
    return false;
}

void OrByte::Widget::_renderDropPreview(const IGraphic& graphic) const
{
    if (!this->_move.active || this->_move.dropTarget == nullptr)
        return;

    Rect area = this->_move.dropTarget->_content;
    const Rect band = cutSide(area, this->_move.dropSide, this->_dockSize(*this->_move.dropTarget, this->_move.dropSide));

    graphic.pushRegion(this->_rect);
    graphic.drawRectangle(Transform{ { band.x + band.w / 2.f, band.y + band.h / 2.f }, 0.f },
        { static_cast<float>(band.w), static_cast<float>(band.h) },
        RGBA{ 90, 150, 255, 90 }, 2, RGBA{ 90, 150, 255, 220 });
    graphic.popRegion();
}

int OrByte::Widget::_effectiveMinSize() const
{
    const bool vertical = resizesVertically(this->_side);
    const Widget *first = this->_children.at(sideIndex(vertical ? Side::Top : Side::Left)).get();
    const Widget *second = this->_children.at(sideIndex(vertical ? Side::Bottom : Side::Right)).get();
    int used = 0;

    if (first != nullptr && first->_visible)
        used += first->_size;
    if (second != nullptr && second->_visible)
        used += second->_size;
    return used + this->_minSize;
}

OrByte::Widget *OrByte::Widget::_rootWidget() noexcept
{
    Widget *widget = this;

    while (widget->_parent != nullptr)
        widget = widget->_parent;
    return widget;
}

const OrByte::Widget *OrByte::Widget::_rootWidget() const noexcept
{
    const Widget *widget = this;

    while (widget->_parent != nullptr)
        widget = widget->_parent;
    return widget;
}

bool OrByte::Widget::isLeftButtonDown() const noexcept
{
    return this->_rootWidget()->_leftDown;
}

void OrByte::Widget::beginDrag(DragPayload payload)
{
    Widget *root = this->_rootWidget();

    root->_drag = Drag{};
    root->_move = Move{};
    root->_payload = Payload{ std::move(payload), nullptr, true };
}

OrByte::Widget *OrByte::Widget::_findAcceptor(int x, int y, const DragPayload& payload)
{
    if (!this->_visible || !this->_rect.contains(x, y))
        return nullptr;

    for (const std::unique_ptr<Widget>& child : this->_children) {
        if (child) {
            if (Widget *found = child->_findAcceptor(x, y, payload))
                return found;
        }
    }
    if (!this->_content.contains(x, y))
        return nullptr;
    return this->canAccept(payload, Vector2(static_cast<float>(x - this->_content.x),
                                            static_cast<float>(y - this->_content.y))) ? this : nullptr;
}

bool OrByte::Widget::_handlePayload(const Event& event)
{
    Payload& payload = this->_payload;

    if (event.type == Event::Type::MouseMoved) {
        payload.hovered = this->_findAcceptor(event.windowX, event.windowY, payload.data);
    } else if (event.type == Event::Type::MouseButtonReleased && event.mouseButton == Event::MouseButton::Left) {
        Widget *target = this->_findAcceptor(event.windowX, event.windowY, payload.data);
        const DragPayload data = payload.data;

        payload = Payload{};
        if (target != nullptr)
            target->onDrop(data, Vector2(static_cast<float>(event.windowX - target->_content.x), static_cast<float>(event.windowY - target->_content.y)));
    }
    return true;
}

void OrByte::Widget::_renderPayload(const IGraphic& graphic) const
{
    if (!this->_payload.active)
        return;

    const DragPayload& data = this->_payload.data;
    const bool valid = this->_payload.hovered != nullptr;
    const float x = static_cast<float>(this->_mouseX) + 14.f;
    float y = static_cast<float>(this->_mouseY) + 14.f;

    graphic.pushRegion(this->_rect);

    if (valid) {
        const Rect& area = this->_payload.hovered->_content;

        graphic.drawRectangle(Transform{ { area.x + area.w / 2.f, area.y + area.h / 2.f }, 0.f },
            { static_cast<float>(area.w), static_cast<float>(area.h) },
            RGBA{ 90, 150, 255, 40 }, 2, RGBA{ 90, 150, 255, 220 });
    }

    if (!data.preview.empty()) {
        graphic.drawImage(Transform{ { x + PREVIEW_SIZE / 2.f, y + PREVIEW_SIZE / 2.f } },
            { PREVIEW_SIZE, PREVIEW_SIZE }, data.preview, true, PREVIEW_OPACITY);
        y += PREVIEW_SIZE + 4.f;
    }

    const Vector2 text = graphic.getTextSize(data.label, FONT_SIZE);
    const float w = text.x + 16.f;
    const float h = text.y + 10.f;

    graphic.drawRectangle(Transform{ { x + w / 2.f, y + h / 2.f }, 0.f }, { w, h },
        RGBA{ 60, 60, 60, 200 }, 1, valid ? RGBA{ 90, 150, 255, 255 } : RGBA{ 120, 120, 120, 255 });
    graphic.drawText(Transform{ { x + 8.f, y + 5.f } }, data.label, FONT_SIZE, 0.f, RGBA{ 255, 255, 255 });

    graphic.popRegion();
}
