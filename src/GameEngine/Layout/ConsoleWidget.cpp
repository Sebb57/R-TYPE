/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ConsoleWidget.cpp
*/

#include "GameEngine/Layout/ConsoleWidget.hpp"
#include "constants.hpp"
#include <algorithm>

namespace {

constexpr OrByte::RGBA BG_COLOR{28, 28, 28, 255};
constexpr OrByte::RGBA INPUT_COLOR{36, 36, 36, 255};
constexpr OrByte::RGBA BORDER_COLOR{80, 80, 80, 255};
constexpr OrByte::RGBA ACCENT_COLOR{90, 150, 255, 255};
constexpr OrByte::RGBA TEXT_COLOR{210, 210, 210, 255};
constexpr OrByte::RGBA INFO_COLOR{140, 140, 140, 255};
constexpr OrByte::RGBA COMMAND_COLOR{130, 180, 255, 255};
constexpr OrByte::RGBA SUCCESS_COLOR{120, 200, 120, 255};
constexpr OrByte::RGBA ERROR_COLOR{230, 110, 100, 255};

constexpr unsigned TEXT_SIZE = 13;
constexpr float LINE_HEIGHT = 17.f;
constexpr float PADDING = 8.f;
constexpr float INPUT_HEIGHT = 26.f;

void fillRect(const OrByte::IGraphic& graphic, float x, float y, float w, float h, OrByte::RGBA color)
{
    graphic.drawRectangle({{x + (w / 2.f), y + (h / 2.f)}, {1.f, 1.f}, 0.f}, {w, h}, color);
}

[[nodiscard]] OrByte::RGBA colorOf(OrByte::Console::LineKind kind)
{
    switch (kind) {
        case OrByte::Console::LineKind::Command:
            return COMMAND_COLOR;
        case OrByte::Console::LineKind::Info:
            return INFO_COLOR;
        case OrByte::Console::LineKind::Success:
            return SUCCESS_COLOR;
        case OrByte::Console::LineKind::Error:
            return ERROR_COLOR;
        default:
            return TEXT_COLOR;
    }
}

} // namespace

OrByte::ConsoleWidget::ConsoleWidget(Console& console, int size) : Widget("Console", size), _console(console)
{
}

void OrByte::ConsoleWidget::draw(const IGraphic& graphic, Vector2u size)
{
    const auto width = static_cast<float>(size.x);
    const auto height = static_cast<float>(size.y);
    const float inputTop = height - INPUT_HEIGHT;
    const auto& lines = this->_console.getLines();
    const auto visible = static_cast<std::size_t>(std::max(0.f, inputTop - PADDING) / LINE_HEIGHT);
    const std::size_t maxScroll = lines.size() > visible ? lines.size() - visible : 0;

    this->_scroll = std::min(this->_scroll, maxScroll);
    fillRect(graphic, 0.f, 0.f, width, height, BG_COLOR);

    const std::size_t end = lines.size() - this->_scroll;
    const std::size_t begin = end > visible ? end - visible : 0;
    float y = PADDING / 2.f;

    for (std::size_t i = begin; i < end; ++i) {
        graphic.drawText({{PADDING, y}}, lines[i].text, TEXT_SIZE, 0.f, colorOf(lines[i].kind));
        y += LINE_HEIGHT;
    }

    const std::string prompt = this->_console.getPrompt();
    const float textY = inputTop + 3.f;

    fillRect(graphic, 0.f, inputTop, width, INPUT_HEIGHT, INPUT_COLOR);
    fillRect(graphic, 0.f, inputTop, width, 1.f, this->hasFocus() ? ACCENT_COLOR : BORDER_COLOR);
    graphic.drawText({{PADDING, textY}}, prompt + this->_input + (this->hasFocus() ? "_" : ""), TEXT_SIZE, 0.f, TEXT_COLOR);
    graphic.drawText({{PADDING, textY}}, prompt, TEXT_SIZE, 0.f, COMMAND_COLOR);
}

bool OrByte::ConsoleWidget::onEvent(const Event& event, [[maybe_unused]] Vector2 localMouse)
{
    switch (event.type) {
        case Event::Type::MouseWheelScrolled:
            if (event.wheelDelta > 0.0)
                this->_scroll += 3;
            else
                this->_scroll -= std::min<std::size_t>(this->_scroll, 3);
            return true;
        case Event::Type::MouseButtonPressed:
            return true;
        case Event::Type::TextEntered:
            if (!this->hasFocus())
                return false;
            if (event.keyChar >= ' ' && event.keyChar <= '~')
                this->_input.push_back(static_cast<char>(event.keyChar));
            return true;
        case Event::Type::KeyPressed:
            return this->hasFocus() && this->_onKey(event);
        case Event::Type::KeyReleased:
            return this->hasFocus();
        default:
            return false;
    }
}

bool OrByte::ConsoleWidget::_onKey(const Event& event)
{
    switch (event.key) {
        case Event::Key::Enter:
            this->_console.execute(this->_input);
            this->_input.clear();
            this->_scroll = 0;
            this->_historyIndex = this->_console.getHistory().size();
            break;
        case Event::Key::Backspace:
            if (!this->_input.empty())
                this->_input.pop_back();
            break;
        case Event::Key::Up:
            this->_browseHistory(-1);
            break;
        case Event::Key::Down:
            this->_browseHistory(1);
            break;
        default:
            break;
    }
    return true;
}

void OrByte::ConsoleWidget::_browseHistory(int direction)
{
    const auto& history = this->_console.getHistory();

    if (direction < 0 && this->_historyIndex > 0)
        --this->_historyIndex;
    else if (direction > 0 && this->_historyIndex < history.size())
        ++this->_historyIndex;
    this->_input = this->_historyIndex < history.size() ? history[this->_historyIndex] : "";
}
