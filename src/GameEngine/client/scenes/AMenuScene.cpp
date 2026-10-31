/**
 * @file AMenuScene.cpp
 * @brief base menu scene implementation
 *
 * @author Seb
 * @date 2026-10-13
**/

#include "GameEngine/client/scenes/AMenuScene.hpp"
#include <utility>
#include "GameEngine/client/network/NetworkClient.hpp"
#include "GameEngine/client/scenes/Theme.hpp"

namespace {

constexpr float RowHeight = 76.0f;
constexpr float RowGap = 12.0f;
constexpr float StatusY = 935.0f;
constexpr float HintsY = 990.0f;

} // namespace

client::AMenuScene::AMenuScene(SceneContext& context) : AScene(context)
{
    this->bindAction(Action::Up, [this](bool pressed) {
        if (pressed)
            this->move(-1);
    });
    this->bindAction(Action::Down, [this](bool pressed) {
        if (pressed)
            this->move(1);
    });
    this->bindAction(Action::Confirm, [this](bool pressed) {
        if (pressed)
            this->activate();
    });
    this->_view = this->_context.network.view();
}

void client::AMenuScene::onEvent(const OrByte::Event& event)
{
    AScene::onEvent(event);
    if (event.type != OrByte::Event::Type::TextEntered || this->_selected >= this->_entries.size())
        return;
    const Entry& entry = this->_entries[this->_selected];
    if (entry.field == nullptr)
        return;
    if (event.keyChar == '\b' && !entry.field->empty())
        entry.field->pop_back();
    else if (event.keyChar >= ' ' && event.keyChar <= '~' && entry.field->size() < entry.maxLength && entry.accepts(static_cast<char>(event.keyChar)))
        entry.field->push_back(static_cast<char>(event.keyChar));
}

void client::AMenuScene::update(double deltaTime)
{
    AScene::update(deltaTime);
    this->_view = this->_context.network.view();
}

void client::AMenuScene::render()
{
    AScene::render();
    if (!this->_entries.empty() && this->_selected >= this->_entries.size())
        this->_selected = this->_entries.size() - 1;
    this->drawText("R-TYPE", Vector2(theme::Margin, 60.0f), 64, theme::Accent);
    this->drawText(this->_subtitle, Vector2(theme::Margin, 165.0f), 30, theme::Muted);
    const auto visible = static_cast<std::size_t>((StatusY - this->_top) / RowHeight);
    const std::size_t first = this->_selected >= visible ? this->_selected - visible + 1 : 0;
    for (std::size_t index = first; index < this->_entries.size() && index < first + visible; ++index)
        this->drawEntry(this->_entries[index], index == this->_selected, this->_top + static_cast<float>(index - first) * RowHeight);
    this->drawText(this->_view.status, Vector2(theme::Margin, StatusY), 26, theme::Muted);
    this->drawText(this->hints(), Vector2(theme::Margin, HintsY), 22, theme::Muted);
}

void client::AMenuScene::addButton(std::string label, Callback onSelect, Condition enabled, std::string detail)
{
    this->_entries.push_back({std::move(label), std::move(detail), std::move(onSelect), std::move(enabled), nullptr, 0, nullptr});
}

void client::AMenuScene::addField(std::string label, std::string& value, std::size_t maxLength, Filter accepts, Callback onSelect)
{
    this->_entries.push_back({std::move(label), "", std::move(onSelect), nullptr, &value, maxLength, std::move(accepts)});
}

void client::AMenuScene::clearEntries()
{
    this->_entries.clear();
}

void client::AMenuScene::move(int step)
{
    if (this->_entries.empty())
        return;
    const auto count = static_cast<int>(this->_entries.size());
    this->_selected = static_cast<std::size_t>((static_cast<int>(this->_selected) + step + count) % count);
}

void client::AMenuScene::activate()
{
    if (this->_selected >= this->_entries.size())
        return;
    const Entry& entry = this->_entries[this->_selected];
    if (entry.onSelect && (!entry.enabled || entry.enabled()))
        entry.onSelect();
}

void client::AMenuScene::drawEntry(const Entry& entry, bool selected, float y) const
{
    const bool enabled = !entry.enabled || entry.enabled();
    const std::string detail = entry.field != nullptr ? *entry.field + (selected ? "_" : "") : entry.detail;
    this->drawPanel(Vector2(theme::Margin, y), Vector2(theme::Width - 2.0f * theme::Margin, RowHeight - RowGap), selected ? theme::RowSelected : theme::Row, selected ? 2 : 0, theme::RowOutline);
    this->drawText(entry.label, Vector2(theme::Margin + 30.0f, y + 14.0f), 30, enabled ? theme::Text : theme::Disabled);
    this->drawText(detail, Vector2(theme::Column, y + 14.0f), 30, enabled ? theme::Muted : theme::Disabled);
}

std::string client::AMenuScene::hints() const
{
    const Controls& controls = this->_context.controls;
    return controls.keyName(Action::Up) + " / " + controls.keyName(Action::Down) + "  move        " + controls.keyName(Action::Confirm) + "  select        " + controls.keyName(Action::Back) + "  back";
}
