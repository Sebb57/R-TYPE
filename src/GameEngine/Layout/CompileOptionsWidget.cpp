/**
 * @file CompileOptionsWidget.cpp
 * @brief compile options tab implementation
 *
 * @author Seb
 * @date 2026-11-11
**/

#include "GameEngine/Layout/CompileOptionsWidget.hpp"
#include "constants.hpp"
#include <algorithm>
#include <exception>
#include <filesystem>
#include <format>
#include <optional>

namespace {

constexpr OrByte::RGBA BG_COLOR{40, 40, 40, 255};
constexpr OrByte::RGBA BUTTON_COLOR{60, 60, 60, 255};
constexpr OrByte::RGBA CHIP_ON_COLOR{52, 62, 74, 255};
constexpr OrByte::RGBA FIELD_COLOR{26, 31, 38, 255};
constexpr OrByte::RGBA BORDER_COLOR{80, 80, 80, 255};
constexpr OrByte::RGBA ACCENT_COLOR{90, 150, 255, 255};
constexpr OrByte::RGBA ACTIVE_BORDER{120, 160, 200, 255};
constexpr OrByte::RGBA TEXT_COLOR{220, 220, 220, 255};
constexpr OrByte::RGBA MUTED_TEXT{150, 158, 170, 255};
constexpr OrByte::RGBA SUCCESS_TEXT{120, 200, 120, 255};
constexpr OrByte::RGBA WARNING_TEXT{220, 170, 90, 255};
constexpr OrByte::RGBA ERROR_TEXT{230, 110, 100, 255};
constexpr OrByte::RGBA TRANSPARENT{0, 0, 0, 0};

constexpr int PADDING = 8;
constexpr int GAP = 6;
constexpr int ROW_HEIGHT = 24;
constexpr int LABEL_WIDTH = 92;
constexpr int FIELD_WIDTH = 260;
constexpr int BUTTON_WIDTH = 72;
constexpr int SMALL_BUTTON_WIDTH = 48;
constexpr int SMALL_BUTTON_HEIGHT = 20;
constexpr int CHIP_HEIGHT = 22;
constexpr int CHECK_SIZE = 12;
constexpr int SCROLL_STEP = 30;
constexpr unsigned TEXT_SIZE = 14;
constexpr unsigned CHIP_TEXT_SIZE = 13;
constexpr float TEXT_CENTER_RATIO = 0.62F;

void fillBox(const OrByte::IGraphic& graphic, const Rect& rect, OrByte::RGBA fill, OrByte::RGBA border = TRANSPARENT)
{
    const auto w = static_cast<float>(rect.w);
    const auto h = static_cast<float>(rect.h);

    graphic.drawRectangle({{static_cast<float>(rect.x) + (w / 2.F), static_cast<float>(rect.y) + (h / 2.F)}, {1.F, 1.F}, 0.F},
        {w, h}, fill, border.a != 0 ? 1 : 0, border);
}

[[nodiscard]] float textTop(const Rect& rect, unsigned size)
{
    return static_cast<float>(rect.y) + (static_cast<float>(rect.h) * 0.5F) - (static_cast<float>(size) * TEXT_CENTER_RATIO);
}

void drawText(const OrByte::IGraphic& graphic, float x, float y, const std::string& text, unsigned size, OrByte::RGBA color)
{
    graphic.drawText({{x, y}, {1.F, 1.F}, 0.F}, text, size, 0.F, color, 0, TRANSPARENT, OrByte::constants::FONT_DEFAULT);
}

void drawButton(const OrByte::IGraphic& graphic, const Rect& rect, const std::string& label, unsigned size, bool highlighted)
{
    const float width = graphic.getTextSize(label, size).x;

    fillBox(graphic, rect, highlighted ? CHIP_ON_COLOR : BUTTON_COLOR, highlighted ? ACCENT_COLOR : BORDER_COLOR);
    drawText(graphic, static_cast<float>(rect.x) + std::max(4.F, (static_cast<float>(rect.w) - width) / 2.F),
        textTop(rect, size), label, size, TEXT_COLOR);
}

[[nodiscard]] bool isBinaryChar(unsigned character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9') || character == '_' || character == '-' || character == '.';
}

[[nodiscard]] const char *sectionTitle(OrByte::CompileOptions::Kind kind)
{
    switch (kind) {
        case OrByte::CompileOptions::Kind::System:
            return "Systems";
        case OrByte::CompileOptions::Kind::Component:
            return "Components";
        case OrByte::CompileOptions::Kind::Metric:
            return "Metrics";
    }
    return "";
}

[[nodiscard]] const char *sectionDirectory(OrByte::CompileOptions::Kind kind)
{
    return kind == OrByte::CompileOptions::Kind::System ? OrByte::constants::SYSTEMS_DIR : OrByte::constants::COMPONENTS_DIR;
}

} // namespace

OrByte::CompileOptionsWidget::CompileOptionsWidget(int size) : Widget("Compile Options", size)
{
    this->reload();
}

void OrByte::CompileOptionsWidget::rescan()
{
    this->_saved.scan(constants::PLUGINS_DIR);
    this->_draft.scan(constants::PLUGINS_DIR);
}

void OrByte::CompileOptionsWidget::reload()
{
    this->_saved = CompileOptions::load(constants::COMPILE_OPTIONS);
    this->_draft = this->_saved;
    this->rescan();
    this->_message.clear();
    this->_editing = false;
}

void OrByte::CompileOptionsWidget::_save()
{
    this->_editing = false;
    if (const std::optional<std::string> error = this->_draft.checkBinaryName(std::filesystem::current_path())) {
        this->_message = *error;
        this->_messageIsError = true;
        return;
    }
    try {
        this->_draft.save(constants::COMPILE_OPTIONS);
    } catch (const std::exception& error) {
        this->_message = std::string("Cannot save: ") + error.what();
        this->_messageIsError = true;
        return;
    }
    this->_saved = this->_draft;
    this->_message = std::string("Saved, used by the next compile");
    this->_messageIsError = false;
}

void OrByte::CompileOptionsWidget::_onClick(Vector2 localMouse)
{
    const auto x = static_cast<int>(localMouse.x);
    const auto y = static_cast<int>(localMouse.y);
    const auto hit = std::ranges::find_if(this->_hits, [x, y](const Hit& candidate) { return candidate.rect.contains(x, y); });

    this->_editing = hit != this->_hits.end() && hit->action == Action::NameField;
    if (hit == this->_hits.end())
        return;
    switch (hit->action) {
        case Action::NameField:
            break;
        case Action::Save:
            this->_save();
            break;
        case Action::Reload:
            this->reload();
            break;
        case Action::EnableAll:
        case Action::DisableAll:
            this->_draft.setAllEnabled(hit->kind, hit->action == Action::EnableAll);
            this->_message.clear();
            break;
        case Action::Toggle:
            this->_draft.setEnabled(hit->kind, hit->name, !this->_draft.isEnabled(hit->kind, hit->name));
            this->_message.clear();
            break;
    }
}

bool OrByte::CompileOptionsWidget::_onKey(const Event& event)
{
    std::string name = this->_draft.getBinaryName();

    switch (event.key) {
        case Event::Key::Enter:
            this->_save();
            return true;
        case Event::Key::Escape:
            this->_editing = false;
            return true;
        case Event::Key::Backspace:
            if (!name.empty()) {
                name.pop_back();
                this->_draft.setBinaryName(std::move(name));
                this->_message.clear();
            }
            return true;
        default:
            return true;
    }
}

bool OrByte::CompileOptionsWidget::onEvent(const Event& event, Vector2 localMouse)
{
    const bool typing = this->_editing && this->hasFocus();

    switch (event.type) {
        case Event::Type::MouseButtonPressed:
            if (event.mouseButton == Event::MouseButton::Left)
                this->_onClick(localMouse);
            return true;
        case Event::Type::MouseWheelScrolled: {
            const int maxScroll = std::max(0, this->_contentHeight - this->_viewHeight);

            this->_scroll = std::clamp(this->_scroll + (event.wheelDelta > 0.0 ? -SCROLL_STEP : SCROLL_STEP), 0, maxScroll);
            return true;
        }
        case Event::Type::TextEntered:
            if (!typing)
                return false;
            if (isBinaryChar(event.keyChar) && this->_draft.getBinaryName().size() < CompileOptions::BINARY_MAX_LENGTH) {
                this->_draft.setBinaryName(this->_draft.getBinaryName() + static_cast<char>(event.keyChar));
                this->_message.clear();
            }
            return true;
        case Event::Type::KeyPressed:
            return typing && this->_onKey(event);
        case Event::Type::KeyReleased:
            return typing;
        default:
            return false;
    }
}

void OrByte::CompileOptionsWidget::_drawHeader(const IGraphic& graphic, int y)
{
    const Rect label(PADDING, y, LABEL_WIDTH, ROW_HEIGHT);
    const Rect field(label.x + label.w, y, FIELD_WIDTH, ROW_HEIGHT);
    const Rect save(field.x + field.w + GAP, y, BUTTON_WIDTH, ROW_HEIGHT);
    const Rect reload(save.x + save.w + GAP, y, BUTTON_WIDTH, ROW_HEIGHT);
    const bool typing = this->_editing && this->hasFocus();
    const bool dirty = this->_isDirty();
    const std::string& name = this->_draft.getBinaryName();
    const std::optional<std::string> nameError = this->_draft.checkBinaryName(std::filesystem::current_path());

    drawText(graphic, static_cast<float>(label.x), textTop(label, TEXT_SIZE), "Binary name", TEXT_SIZE, TEXT_COLOR);
    fillBox(graphic, field, FIELD_COLOR, typing ? ACTIVE_BORDER : BORDER_COLOR);
    drawText(graphic, static_cast<float>(field.x + GAP), textTop(field, TEXT_SIZE), name + (typing ? "|" : ""), TEXT_SIZE,
        name.empty() ? MUTED_TEXT : TEXT_COLOR);
    drawButton(graphic, save, "Save", TEXT_SIZE, dirty);
    drawButton(graphic, reload, "Reload", TEXT_SIZE, false);
    this->_hits.emplace_back(field, Action::NameField);
    this->_hits.emplace_back(save, Action::Save);
    this->_hits.emplace_back(reload, Action::Reload);

    std::string status = this->_message;
    RGBA color = this->_messageIsError ? ERROR_TEXT : SUCCESS_TEXT;

    if (status.empty() && nameError) {
        status = *nameError;
        color = ERROR_TEXT;
    } else if (status.empty() && dirty) {
        status = "Unsaved changes";
        color = WARNING_TEXT;
    }
    drawText(graphic, static_cast<float>(reload.x + reload.w + PADDING), textTop(reload, TEXT_SIZE), status, TEXT_SIZE, color);
}

int OrByte::CompileOptionsWidget::_drawSection(const IGraphic& graphic, CompileOptions::Kind kind, int y, int width)
{
    const std::vector<std::string>& plugins = this->_draft.getPlugins(kind);
    const std::size_t enabled = this->_draft.getEnabled(kind).size();
    const std::string title = std::format("{}  {}/{} compiled", sectionTitle(kind), enabled, plugins.size());
    const Rect titleRect(PADDING, y, 0, SMALL_BUTTON_HEIGHT);
    const int titleWidth = static_cast<int>(graphic.getTextSize(title, TEXT_SIZE).x);
    const Rect all(PADDING + titleWidth + PADDING, y, SMALL_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);
    const Rect none(all.x + all.w + GAP, y, SMALL_BUTTON_WIDTH, SMALL_BUTTON_HEIGHT);

    drawText(graphic, static_cast<float>(PADDING), textTop(titleRect, TEXT_SIZE), title, TEXT_SIZE, TEXT_COLOR);
    drawButton(graphic, all, "All", CHIP_TEXT_SIZE, false);
    drawButton(graphic, none, "None", CHIP_TEXT_SIZE, false);
    this->_hits.emplace_back(all, Action::EnableAll, kind);
    this->_hits.emplace_back(none, Action::DisableAll, kind);
    y += SMALL_BUTTON_HEIGHT + GAP;

    if (plugins.empty()) {
        const Rect line(PADDING, y, 0, CHIP_HEIGHT);

        drawText(graphic, static_cast<float>(PADDING), textTop(line, CHIP_TEXT_SIZE),
            std::format("No plugin in {}, build the project first: everything will be compiled", sectionDirectory(kind)),
            CHIP_TEXT_SIZE, MUTED_TEXT);
        return y + CHIP_HEIGHT + PADDING;
    }

    int x = PADDING;

    for (const std::string& name : plugins) {
        const bool on = this->_draft.isEnabled(kind, name);
        const int chipWidth = GAP + CHECK_SIZE + GAP + static_cast<int>(graphic.getTextSize(name, CHIP_TEXT_SIZE).x) + PADDING;

        if (x > PADDING && x + chipWidth > width - PADDING) {
            x = PADDING;
            y += CHIP_HEIGHT + GAP;
        }

        const Rect chip(x, y, chipWidth, CHIP_HEIGHT);
        const Rect check(x + GAP, y + ((CHIP_HEIGHT - CHECK_SIZE) / 2), CHECK_SIZE, CHECK_SIZE);

        fillBox(graphic, chip, on ? CHIP_ON_COLOR : BUTTON_COLOR, on ? ACCENT_COLOR : BORDER_COLOR);
        fillBox(graphic, check, on ? ACCENT_COLOR : FIELD_COLOR, on ? ACCENT_COLOR : BORDER_COLOR);
        drawText(graphic, static_cast<float>(check.x + check.w + GAP), textTop(chip, CHIP_TEXT_SIZE), name, CHIP_TEXT_SIZE,
            on ? TEXT_COLOR : MUTED_TEXT);
        this->_hits.emplace_back(chip, Action::Toggle, kind, name);
        x += chipWidth + GAP;
    }
    return y + CHIP_HEIGHT + (2 * PADDING);
}

void OrByte::CompileOptionsWidget::draw(const IGraphic& graphic, Vector2u size)
{
    const int width = static_cast<int>(size.x);

    this->_viewHeight = static_cast<int>(size.y);
    this->_scroll = std::clamp(this->_scroll, 0, std::max(0, this->_contentHeight - this->_viewHeight));
    this->_hits.clear();
    fillBox(graphic, Rect(0, 0, width, this->_viewHeight), BG_COLOR);

    int y = PADDING - this->_scroll;

    this->_drawHeader(graphic, y);
    y += ROW_HEIGHT + (2 * PADDING);
    y = this->_drawSection(graphic, CompileOptions::Kind::System, y, width);
    y = this->_drawSection(graphic, CompileOptions::Kind::Component, y, width);
    y = this->_drawSection(graphic, CompileOptions::Kind::Metric, y, width);
    this->_contentHeight = y + this->_scroll;
}
