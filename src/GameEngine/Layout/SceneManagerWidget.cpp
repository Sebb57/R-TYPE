#include "GameEngine/Layout/SceneManagerWidget.hpp"
#include "constants.hpp"
#include <algorithm>
#include <string>

namespace {
constexpr int Padding = 6;
constexpr int TabWidth = 180;
constexpr int TabStride = TabWidth + Padding;
constexpr int AddButtonWidth = 26;
constexpr int CloseButtonWidth = 22;

struct SceneLayout {
    Rect row;
    Rect addButton;
    std::size_t visibleTabs;

    [[nodiscard]] Rect tab(std::size_t slot) const
    {
        return {row.x + static_cast<int>(slot) * TabStride, row.y, row.w, row.h};
    }
};

SceneLayout makeLayout(const Rect& content, int headerHeight)
{
    const int top = headerHeight + Padding;
    const int height = std::max(0, content.h - headerHeight - 2 * Padding);
    return {
        {Padding, top, std::clamp(content.w - 3 * Padding - AddButtonWidth, 0, TabWidth), height},
        {std::max(0, content.w - Padding - AddButtonWidth), top, std::min(AddButtonWidth, content.w), height},
        static_cast<std::size_t>(std::max(1, (content.w - 2 * Padding - AddButtonWidth) / TabStride)),
    };
}

std::size_t maxScrollOffset(std::size_t count, std::size_t visible)
{
    return count > visible ? count - visible : 0;
}

void drawBox(const OrByte::IGraphic& graphic, const Rect& rect, bool selected)
{
    graphic.drawRectangle({{rect.x + rect.w / 2.f, rect.y + rect.h / 2.f}, 0.f},
        {static_cast<float>(rect.w), static_cast<float>(rect.h)},
        selected ? OrByte::RGBA(52, 62, 74, 255) : OrByte::RGBA(44, 47, 52, 255), 1,
        selected ? OrByte::RGBA(120, 160, 200, 255) : OrByte::RGBA(80, 80, 80, 255));
}

void drawLabel(const OrByte::IGraphic& graphic, float x, float y, const std::string& text, unsigned size = 14)
{
    graphic.drawText({{x, y}, 0.f}, text, size, 0.f, OrByte::RGBA(220, 220, 220, 255), 0,
        OrByte::RGBA(0, 0, 0, 0), OrByte::constants::FONT_DEFAULT);
}

} // namespace

OrByte::SceneManagerWidget::SceneManagerWidget(SceneCollection& scenes)
    : Panel("Scenes", 64), _scenes(scenes)
{
}

void OrByte::SceneManagerWidget::_revealSelection(std::size_t visibleTabs)
{
    const auto selected = this->_scenes.selectedIndex();
    if (selected < this->_scrollOffset)
        this->_scrollOffset = selected;
    else if (selected >= this->_scrollOffset + visibleTabs)
        this->_scrollOffset = selected - visibleTabs + 1;
}

void OrByte::SceneManagerWidget::draw(const IGraphic& graphic, Vector2u size)
{
    Panel::draw(graphic, size);
    const auto layout = makeLayout(this->getContentRect(), this->getHeaderHeight());
    const auto& scenes = this->_scenes.scenes();
    const auto selected = this->_scenes.selectedIndex();
    if (this->_lastSelected != selected || this->_lastWidth != size.x) {
        this->_revealSelection(layout.visibleTabs);
        this->_lastSelected = selected;
        this->_lastWidth = size.x;
    }
    this->_scrollOffset = std::min(this->_scrollOffset, maxScrollOffset(scenes.size(), layout.visibleTabs));

    for (std::size_t slot = 0; slot < layout.visibleTabs && this->_scrollOffset + slot < scenes.size(); ++slot) {
        const auto index = this->_scrollOffset + slot;
        const Rect tab = layout.tab(slot);
        const float textY = tab.y + tab.h / 2.f - 9;
        std::string label = scenes[index].path.empty() ? "New Scene" : scenes[index].path.stem().string();
        const auto maxChars = static_cast<std::size_t>(std::max(0, (tab.w - CloseButtonWidth - 20) / 8));
        if (label.size() > maxChars)
            label = maxChars > 3 ? label.substr(0, maxChars - 3) + "..." : label.substr(0, maxChars);
        drawBox(graphic, tab, index == selected);
        drawLabel(graphic, tab.x + 10.f, textY, label);
        drawLabel(graphic, tab.x + tab.w - CloseButtonWidth + 6.f, textY, "x");
    }
    const Rect& add = layout.addButton;
    drawBox(graphic, add, true);
    drawLabel(graphic, add.x + add.w / 2.f - 5, add.y + add.h / 2.f - 11, "+", 18);
}

bool OrByte::SceneManagerWidget::onEvent(const Event& event, Vector2 localMouse)
{
    const auto layout = makeLayout(this->getContentRect(), this->getHeaderHeight());
    const auto count = this->_scenes.scenes().size();
    if (event.type == Event::Type::MouseWheelScrolled) {
        if (event.wheelDelta > 0 && this->_scrollOffset > 0)
            --this->_scrollOffset;
        else if (event.wheelDelta < 0 && this->_scrollOffset < maxScrollOffset(count, layout.visibleTabs))
            ++this->_scrollOffset;
        return true;
    }
    if (event.type != Event::Type::MouseButtonPressed || event.mouseButton != Event::MouseButton::Left)
        return false;
    const auto x = static_cast<int>(localMouse.x);
    const auto y = static_cast<int>(localMouse.y);
    if (layout.addButton.contains(x, y)) {
        this->_scenes.create();
        return true;
    }
    for (std::size_t slot = 0; slot < layout.visibleTabs && this->_scrollOffset + slot < count; ++slot) {
        const auto index = this->_scrollOffset + slot;
        const Rect tab = layout.tab(slot);
        if (!tab.contains(x, y))
            continue;
        if (x >= tab.x + tab.w - CloseButtonWidth)
            this->_scenes.close(index);
        else
            this->_scenes.select(index);
        return true;
    }
    return true;
}
