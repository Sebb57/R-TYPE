/*
** EPITECH PROJECT, 2026
** RType
** File description:
** EntityWidget.cpp
*/

#include "GameEngine/Layout/EntityWidget.hpp"
#include "GameEngine/ECS/Components/Name.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <string>

namespace {

constexpr float TitleHeight = 24.0f;
constexpr float MinimumRowHeight = 32.0f;

struct EntityLayout {
    float headerHeight;
    float rowHeight;
    float padding;
    float idColumnWidth;
    float rowTextSize;
    std::size_t visibleRows;
};

[[nodiscard]] EntityLayout makeLayout(const Rect& bounds)
{
    const float width = std::max(1.0f, static_cast<float>(bounds.w));
    const float height = std::max(1.0f, static_cast<float>(bounds.h));
    const float headerHeight = std::min(height, TitleHeight);
    const float rowsHeight = height - headerHeight;
    const auto visibleRows = std::max<std::size_t>(1, static_cast<std::size_t>(
        std::floor(rowsHeight / MinimumRowHeight)));
    const float rowHeight = std::max(1.0f, rowsHeight / static_cast<float>(visibleRows));
    const float padding = std::max(8.0f, width * 0.04f);
    const float idColumnWidth = std::max(34.0f, width * 0.18f);
    const float rowTextSize = std::clamp(rowHeight * 0.5f, 11.0f, 14.0f);

    return {headerHeight, rowHeight, padding, idColumnWidth, rowTextSize, visibleRows};
}

[[nodiscard]] float rowTopIndex(std::size_t index, const EntityLayout& layout)
{
    return layout.headerHeight + static_cast<float>(index) * layout.rowHeight;
}

[[nodiscard]] int rowIndexAt(float localY, const EntityLayout& layout)
{
    if (localY < layout.headerHeight)
        return -1;

    const auto index = static_cast<std::size_t>((localY - layout.headerHeight) / layout.rowHeight);
    return index < layout.visibleRows ? static_cast<int>(index) : -1;
}

[[nodiscard]] std::size_t maxScrollOffset(std::size_t entityCount, std::size_t visibleRows)
{
    return entityCount > visibleRows ? entityCount - visibleRows : 0;
}

} // namespace

OrByte::EntityWidget::EntityWidget(World& world, int width)
    : Panel("Hierarchy", width), _world(world)
{
}

std::vector<int> OrByte::EntityWidget::_entityIds() const
{
    return this->_world.getExistingEntities();
}

std::string OrByte::EntityWidget::_nameOf(int entityId) const
{
    const auto *name = this->_world.getComponent<OrByte::Name>(entityId);
    return name ? name->name : "Entity_" + std::to_string(entityId);
}

void OrByte::EntityWidget::_setSelectedIndex(std::size_t index)
{
    const auto ids = this->_entityIds();
    if (ids.empty())
        return;

    if (this->_editing && this->_editingEntityId >= 0)
        this->_finishEditing();

    this->_selectedIndex = std::min(index, ids.size() - 1);
    const int selectedEntityId = ids[this->_selectedIndex];
    this->_world.setEntitySelected(selectedEntityId);
}

void OrByte::EntityWidget::_startEditing(int entityId)
{
    this->_editing = true;
    this->_editingEntityId = entityId;
    this->_editingText = this->_nameOf(entityId);
    this->_world.setEntitySelected(entityId);
}

void OrByte::EntityWidget::_finishEditing()
{
    if (!this->_editing)
        return;

    if (this->_editingEntityId >= 0) {
        if (auto *name = this->_world.getComponent<OrByte::Name>(this->_editingEntityId))
            name->name = this->_editingText.empty()
                ? "Entity_" + std::to_string(this->_editingEntityId)
                : this->_editingText;
    }

    this->_editing = false;
    this->_editingEntityId = -1;
    this->_editingText.clear();
}

void OrByte::EntityWidget::_cancelEditing()
{
    this->_editing = false;
    this->_editingEntityId = -1;
    this->_editingText.clear();
}

void OrByte::EntityWidget::_applyTextInput(const Event& event)
{
    if (!this->_editing)
        return;

    if (event.keyChar == '\n' || event.keyChar == '\r') {
        this->_finishEditing();
        return;
    }
    if (event.keyChar == static_cast<unsigned>(27)) {
        this->_cancelEditing();
        return;
    }
    if (event.keyChar >= 32 && event.keyChar <= 126)
        this->_editingText.push_back(static_cast<char>(event.keyChar));
}

void OrByte::EntityWidget::_applyKeyAction(const Event& event)
{
    if (event.key == Event::Key::Delete ||
        (event.key == Event::Key::Backspace && !this->_editing)) {
        const auto ids = this->_entityIds();
        const std::optional<int> selectedEntity = this->_world.getSelectedEntity();
        if (ids.empty() || !selectedEntity)
            return;

        const auto selected = std::find(ids.begin(), ids.end(), *selectedEntity);
        if (selected == ids.end() || this->_world.getCameraEntity() == *selectedEntity)
            return;
        const std::size_t index = static_cast<std::size_t>(std::distance(ids.begin(), selected));
        this->_cancelEditing();
        this->_world.destroyEntity(*selectedEntity);

        const auto remainingIds = this->_entityIds();
        if (remainingIds.empty()) {
            this->_selectedIndex = 0;
            this->_scrollOffset = 0;
            this->_world.clearSelectedEntity();
            return;
        }

        this->_selectedIndex = std::min(index, remainingIds.size() - 1);
        this->_world.setEntitySelected(remainingIds[this->_selectedIndex]);
        const auto metrics = makeLayout(this->getContentRect());
        const std::size_t maxScroll = maxScrollOffset(remainingIds.size(), metrics.visibleRows);
        this->_scrollOffset = std::clamp(this->_scrollOffset, std::size_t{0}, maxScroll);
        if (this->_selectedIndex < this->_scrollOffset)
            this->_scrollOffset = this->_selectedIndex;
        else if (this->_selectedIndex >= this->_scrollOffset + metrics.visibleRows)
            this->_scrollOffset = this->_selectedIndex - metrics.visibleRows + 1;
        return;
    }

    if (this->_editing) {
        if (event.key == Event::Key::Backspace) {
            if (!this->_editingText.empty())
                this->_editingText.pop_back();
            return;
        }
        if (event.key == Event::Key::Escape) {
            this->_cancelEditing();
            return;
        }
        if (event.key == Event::Key::Enter) {
            this->_finishEditing();
            return;
        }
        return;
    }

    if (event.key == Event::Key::Enter) {
        const auto ids = this->_entityIds();
        if (!ids.empty())
            this->_startEditing(ids[this->_selectedIndex]);
    }
}

void OrByte::EntityWidget::_selectRow(Vector2 localMouse)
{
    const auto ids = this->_entityIds();
    if (ids.empty())
        return;

    const auto content = this->getContentRect();
    const auto metrics = makeLayout(content);
    const int rowIndex = rowIndexAt(localMouse.y, metrics);
    if (rowIndex < 0 || rowIndex >= static_cast<int>(std::min(ids.size(), metrics.visibleRows)))
        return;

    const std::size_t target = static_cast<std::size_t>(rowIndex) + this->_scrollOffset;
    if (target >= ids.size())
        return;

    if (target == this->_selectedIndex) {
        if (this->_editing)
            this->_finishEditing();
        else
            this->_startEditing(ids[target]);
        return;
    }

    this->_setSelectedIndex(target);
}

void OrByte::EntityWidget::_addEntity()
{
    if (this->_editing)
        this->_finishEditing();

    const int entityId = this->_world.createEntity();
    this->_world.setEntitySelected(entityId);
}

void OrByte::EntityWidget::draw(const IGraphic& graphic, Vector2u size)
{
    const auto ids = this->_entityIds();
    const std::optional<int> selectedEntityId = this->_world.getSelectedEntity();
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const auto metrics = makeLayout(Rect(0, 0, width, height));
    const auto selectedIt = selectedEntityId
        ? std::find(ids.begin(), ids.end(), *selectedEntityId) : ids.end();
    if (selectedIt != ids.end()) {
        this->_selectedIndex = static_cast<std::size_t>(std::distance(ids.begin(), selectedIt));
        const std::size_t maxScroll = maxScrollOffset(ids.size(), metrics.visibleRows);
        this->_scrollOffset = std::clamp(this->_scrollOffset, std::size_t{0}, maxScroll);
        if (this->_selectedIndex < this->_scrollOffset)
            this->_scrollOffset = this->_selectedIndex;
        else if (this->_selectedIndex >= this->_scrollOffset + metrics.visibleRows)
            this->_scrollOffset = this->_selectedIndex - metrics.visibleRows + 1;
    }

    const RGBA bgColor = RGBA{40, 40, 40, 255};
    const RGBA headerColor = RGBA{60, 60, 60, 255};
    const RGBA rowColor = RGBA{26, 31, 38};
    const RGBA selectedColor = RGBA{52, 62, 74};
    const RGBA borderColor = RGBA{80, 80, 80, 255};
    const RGBA textColor = RGBA{220, 220, 220, 255};
    const RGBA mutedText = RGBA{170, 180, 196};

    const float centerX = static_cast<float>(width) * 0.5f;
    const float centerY = static_cast<float>(height) * 0.5f;
    const float headerCenterY = static_cast<float>(metrics.headerHeight) * 0.5f;

    graphic.drawRectangle({{centerX, centerY}, {1.0f, 1.0f}, 0.0f},
        {static_cast<float>(width), static_cast<float>(height)}, bgColor, 1, borderColor);
    graphic.drawRectangle({{centerX, headerCenterY}, {1.0f, 1.0f}, 0.0f},
        {static_cast<float>(width), static_cast<float>(metrics.headerHeight)}, headerColor, 0, RGBA{0, 0, 0});
    graphic.drawText({{6.0f, 3.0f}, {1.0f, 1.0f}, 0.0f}, this->getTitle(), 14, 0.f, textColor, 0, RGBA{0, 0, 0, 0}, constants::FONT_DEFAULT);

    const float addButtonWidth = std::min(22.0f, static_cast<float>(width));
    const float addButtonX = static_cast<float>(width) - addButtonWidth - 2.0f;
    const float addButtonHeight = std::max(0.0f, metrics.headerHeight - 4.0f);
    const float addButtonY = (metrics.headerHeight - addButtonHeight) * 0.5f;
    graphic.drawRectangle({{addButtonX + addButtonWidth * 0.5f, addButtonY + addButtonHeight * 0.5f},
        {1.0f, 1.0f}, 0.0f}, {addButtonWidth, addButtonHeight}, RGBA{76, 92, 108}, 1, borderColor);
    graphic.drawText({{addButtonX + addButtonWidth * 0.5f - 4.0f, addButtonY}, {1.0f, 1.0f}, 0.0f},
        "+", 16, 0.f, textColor, 0, RGBA{0, 0, 0, 0}, constants::FONT_DEFAULT);

    const std::size_t rowsToDraw = std::min<std::size_t>(ids.size(), metrics.visibleRows);
    for (std::size_t index = 0; index < rowsToDraw; ++index) {
        const std::size_t entityIndex = this->_scrollOffset + index;
        if (entityIndex >= ids.size())
            break;

        const int entityId = ids[entityIndex];
        const bool selected = selectedEntityId && entityId == *selectedEntityId;
        const bool editingThis = this->_editing && this->_editingEntityId == entityId;
        const float rowTop = rowTopIndex(index, metrics);
        const float rowBottom = rowTop + metrics.rowHeight;
        const float textY = rowTop + (metrics.rowHeight - metrics.rowTextSize) * 0.5f + 1.0f;
        const float labelX = metrics.padding;
        const float idX = std::max(0.0f, width - metrics.idColumnWidth - metrics.padding);
        const float rowCenterY = rowTop + metrics.rowHeight * 0.5f;

        const RGBA rowFill = selected ? selectedColor : rowColor;
        const RGBA outline = editingThis ? RGBA{120, 160, 200} : borderColor;
        const std::string label = editingThis ? this->_editingText : this->_nameOf(entityId);

        if (rowBottom > static_cast<float>(height))
            break;

        graphic.drawRectangle({{centerX, rowCenterY}, {1.0f, 1.0f}, 0.0f},
            {static_cast<float>(width), metrics.rowHeight}, rowFill, 1, outline);
        graphic.drawText({{labelX, textY}, {1.0f, 1.0f}, 0.0f}, label, static_cast<unsigned>(metrics.rowTextSize), 0.f, textColor, 0, RGBA{0, 0, 0}, constants::FONT_DEFAULT);
        graphic.drawText({{idX, textY + 1.0f}, {1.0f, 1.0f}, 0.0f}, "id: " + std::to_string(entityId), static_cast<unsigned>(std::max(10.0f, metrics.rowTextSize * 0.85f)), 0.f, mutedText, 0, RGBA{0, 0, 0}, constants::FONT_DEFAULT);
    }

    if (ids.size() > metrics.visibleRows) {
        const float trackTop = metrics.headerHeight + 3.0f;
        const float trackHeight = std::max(0.0f, static_cast<float>(height) - trackTop - 3.0f);
        const float barWidth = 5.0f;
        const float barX = static_cast<float>(width) - barWidth - 3.0f;
        const float thumbHeight = std::min(trackHeight, std::max(18.0f,
            trackHeight * static_cast<float>(metrics.visibleRows) / static_cast<float>(ids.size())));
        const std::size_t maxScroll = maxScrollOffset(ids.size(), metrics.visibleRows);
        const float scrollProgress = maxScroll == 0 ? 0.0f
            : static_cast<float>(std::min(this->_scrollOffset, maxScroll)) / static_cast<float>(maxScroll);
        const float thumbTop = trackTop + (trackHeight - thumbHeight) * scrollProgress;

        graphic.drawRectangle({{barX + barWidth * 0.5f, trackTop + trackHeight * 0.5f}, {1.0f, 1.0f}, 0.0f},
            {barWidth, trackHeight}, RGBA{38, 44, 52}, 0, RGBA{0, 0, 0});
        graphic.drawRectangle({{barX + barWidth * 0.5f, thumbTop + thumbHeight * 0.5f}, {1.0f, 1.0f}, 0.0f},
            {barWidth, thumbHeight}, RGBA{120, 140, 160}, 0, RGBA{0, 0, 0});
    }
}

bool OrByte::EntityWidget::onEvent(const Event& event, Vector2 localMouse)
{
    if (event.type == Event::Type::MouseWheelScrolled) {
        const auto ids = this->_entityIds();
        if (ids.empty())
            return true;

        const auto content = this->getContentRect();
        const auto metrics = makeLayout(content);
        const std::size_t maxScroll = maxScrollOffset(ids.size(), metrics.visibleRows);
        this->_scrollOffset = std::clamp(this->_scrollOffset, std::size_t{0}, maxScroll);

        if (event.wheelDelta > 0.0) {
            if (this->_scrollOffset > 0)
                --this->_scrollOffset;
        } else if (event.wheelDelta < 0.0) {
            if (this->_scrollOffset < maxScroll)
                ++this->_scrollOffset;
        }
        return true;
    }

    if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Left) {
        const auto content = this->getContentRect();
        const float addButtonLeft = static_cast<float>(content.w) - 24.0f;
        if (localMouse.y < TitleHeight && localMouse.x >= addButtonLeft &&
            localMouse.x < static_cast<float>(content.w)) {
            this->_addEntity();
            return true;
        }

        const auto ids = this->_entityIds();
        const auto metrics = makeLayout(content);
        const float trackTop = metrics.headerHeight + 3.0f;
        const float trackHeight = std::max(0.0f, static_cast<float>(content.h) - trackTop - 3.0f);
        const float barX = static_cast<float>(content.w) - 8.0f;
        if (ids.size() > metrics.visibleRows && localMouse.x >= barX - 4.0f &&
            localMouse.x < static_cast<float>(content.w) && localMouse.y >= trackTop &&
            localMouse.y <= trackTop + trackHeight) {
            return true;
        }
        const int rowIndex = rowIndexAt(localMouse.y, metrics);
        const bool clickedRow = rowIndex >= 0 && rowIndex < static_cast<int>(
            std::min(ids.size(), metrics.visibleRows));
        this->_selectRow(localMouse);
        if (clickedRow) {
            if (const std::optional<int> selected = this->_world.getSelectedEntity())
                this->_world.setPendingEntityDrag(*selected);
        }
        return true;
    }
    if (event.type == Event::Type::TextEntered) {
        this->_applyTextInput(event);
        return true;
    }
    if (event.type == Event::Type::KeyPressed) {
        this->_applyKeyAction(event);
        return true;
    }
    return false;
}
