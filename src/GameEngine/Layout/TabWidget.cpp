/*
** EPITECH PROJECT, 2026
** RType
** File description:
** TabWidget.cpp
*/

#include "GameEngine/Layout/TabWidget.hpp"
#include "constants.hpp"
#include <stdexcept>
#include <utility>

namespace {

constexpr OrByte::RGBA BG_COLOR{40, 40, 40, 255};
constexpr OrByte::RGBA HEADER_COLOR{60, 60, 60, 255};
constexpr OrByte::RGBA BORDER_COLOR{80, 80, 80, 255};
constexpr OrByte::RGBA TEXT_COLOR{220, 220, 220, 255};
constexpr OrByte::RGBA MUTED_TEXT{160, 160, 160, 255};
constexpr OrByte::RGBA ACCENT_COLOR{90, 150, 255, 255};

constexpr unsigned TEXT_SIZE = 14;
constexpr float TAB_PADDING = 12.f;
constexpr float TEXT_TOP = 4.f;
constexpr float ACCENT_HEIGHT = 2.f;

void fillRect(const OrByte::IGraphic& graphic, float x, float y, float w, float h, OrByte::RGBA color)
{
    graphic.drawRectangle({{x + (w / 2.f), y + (h / 2.f)}, {1.f, 1.f}, 0.f}, {w, h}, color, 0, OrByte::RGBA{0, 0, 0, 0});
}

} // namespace

OrByte::TabWidget::TabWidget(int size) : Widget("", size)
{
    this->setMinSize(TAB_HEIGHT * 3);
}

OrByte::Widget& OrByte::TabWidget::addTab(std::unique_ptr<Widget> page)
{
    if (!page)
        throw std::invalid_argument("TabWidget::addTab: null page");

    Widget& added = *page;

    this->_tabs.push_back(Tab{added.getTitle(), &added, std::move(page)});
    added.setTitle("");
    if (this->getBody() == nullptr) {
        this->_selected = this->_tabs.size() - 1;
        this->setBody(std::move(this->_tabs.back().stored));
    }
    return added;
}

void OrByte::TabWidget::select(std::size_t index)
{
    if (index >= this->_tabs.size() || (index == this->_selected && this->getBody() != nullptr))
        return;
    this->_tabs.at(this->_selected).stored = this->takeBody();
    this->_selected = index;
    this->setBody(std::move(this->_tabs.at(index).stored));
}

void OrByte::TabWidget::select(const Widget& page)
{
    for (std::size_t i = 0; i < this->_tabs.size(); ++i)
        if (this->_tabs[i].page == &page) {
            this->select(i);
            return;
        }
}

void OrByte::TabWidget::draw(const IGraphic& graphic, Vector2u size)
{
    const auto width = static_cast<float>(size.x);
    const auto height = static_cast<float>(size.y);
    const auto header = static_cast<float>(TAB_HEIGHT);
    float x = 0.f;

    fillRect(graphic, 0.f, 0.f, width, height, BG_COLOR);
    fillRect(graphic, 0.f, 0.f, width, header, HEADER_COLOR);
    fillRect(graphic, 0.f, header - 1.f, width, 1.f, BORDER_COLOR);
    for (std::size_t i = 0; i < this->_tabs.size(); ++i) {
        Tab& tab = this->_tabs[i];
        const bool active = i == this->_selected;
        const Vector2 text = graphic.getTextSize(tab.label, TEXT_SIZE);

        tab.x = x;
        tab.width = text.x + (2.f * TAB_PADDING);
        if (active) {
            fillRect(graphic, tab.x, 0.f, tab.width, header, BG_COLOR);
            fillRect(graphic, tab.x, 0.f, tab.width, ACCENT_HEIGHT, ACCENT_COLOR);
        }
        graphic.drawText({{tab.x + TAB_PADDING, TEXT_TOP}, {1.f, 1.f}, 0.f}, tab.label, TEXT_SIZE, 0.f,
            active ? TEXT_COLOR : MUTED_TEXT, 0, RGBA{0, 0, 0, 0}, constants::FONT_DEFAULT);
        x += tab.width;
        fillRect(graphic, x, 0.f, 1.f, header, BORDER_COLOR);
        x += 1.f;
    }
}

bool OrByte::TabWidget::onEvent(const Event& event, Vector2 localMouse)
{
    if (event.type != Event::Type::MouseButtonPressed || localMouse.y >= static_cast<float>(TAB_HEIGHT))
        return false;
    if (event.mouseButton == Event::MouseButton::Left)
        for (std::size_t i = 0; i < this->_tabs.size(); ++i)
            if (localMouse.x >= this->_tabs[i].x && localMouse.x < this->_tabs[i].x + this->_tabs[i].width) {
                this->select(i);
                break;
            }
    return true;
}
