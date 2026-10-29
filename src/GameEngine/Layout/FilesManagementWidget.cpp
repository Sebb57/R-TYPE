/*
** EPITECH PROJECT, 2026
** RType
** File description:
** FilesManagementWidget.cpp
*/

#include "GameEngine/Layout/FilesManagementWidget.hpp"
#include "constants.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>

namespace {

enum class Button : std::uint8_t {
    Compile,
    Save,
    Load,
};

constexpr std::array<const char *, 3> ButtonLabels{"Compile", "Save", "Load"};
constexpr int ButtonCount = static_cast<int>(ButtonLabels.size());

constexpr int Padding = 6;
constexpr int ButtonWidth = 90;
constexpr int FileButtonWidth = 210;
constexpr unsigned ButtonTextSize = 14;
constexpr unsigned FileTextSize = 12;
// IGraphic cannot measure text, glyphs are assumed to be ~0.55 times the font size wide
constexpr float GlyphWidthRatio = 0.55f;

const OrByte::RGBA ButtonColor{60, 60, 60, 255};
const OrByte::RGBA ActiveColor{52, 62, 74, 255};
const OrByte::RGBA BorderColor{80, 80, 80, 255};
const OrByte::RGBA ActiveBorderColor{120, 160, 200, 255};
const OrByte::RGBA TextColor{220, 220, 220, 255};
const OrByte::RGBA MutedText{170, 180, 196, 255};

[[nodiscard]] Rect buttonRect(Button button, int height)
{
    const int index = static_cast<int>(button);

    return {Padding + index * (ButtonWidth + Padding), Padding, ButtonWidth, std::max(0, height - 2 * Padding)};
}

[[nodiscard]] constexpr int pickerX()
{
    return Padding + ButtonCount * (ButtonWidth + Padding) + Padding;
}

[[nodiscard]] Rect fileRect(std::size_t slot, int height)
{
    return {pickerX() + static_cast<int>(slot) * (FileButtonWidth + Padding), Padding, FileButtonWidth, std::max(0, height - 2 * Padding)};
}

[[nodiscard]] std::size_t visibleFiles(int width)
{
    const int room = width - pickerX();

    return room > 0 ? static_cast<std::size_t>((room + Padding) / (FileButtonWidth + Padding)) : 0;
}

[[nodiscard]] float textWidth(const std::string& text, unsigned size)
{
    return static_cast<float>(text.size()) * static_cast<float>(size) * GlyphWidthRatio;
}

[[nodiscard]] std::string fitText(const std::string& text, unsigned size, int width)
{
    const auto maxChars = static_cast<std::size_t>(static_cast<float>(width) / (static_cast<float>(size) * GlyphWidthRatio));

    if (text.size() <= maxChars || maxChars < 3)
        return text;
    return text.substr(0, maxChars - 3) + "...";
}

void drawButton(const OrByte::IGraphic& graphic, const Rect& rect, const std::string& label, unsigned textSize, bool active)
{
    const auto w = static_cast<float>(rect.w);
    const auto h = static_cast<float>(rect.h);
    const float textX = static_cast<float>(rect.x) + std::max(4.0f, (w - textWidth(label, textSize)) * 0.5f);
    const float textY = static_cast<float>(rect.y) + (h - static_cast<float>(textSize)) * 0.5f - 2.0f;

    graphic.drawRectangle({{static_cast<float>(rect.x) + w * 0.5f, static_cast<float>(rect.y) + h * 0.5f}, {1.0f, 1.0f}, 0.0f},
        {w, h}, active ? ActiveColor : ButtonColor, 1, active ? ActiveBorderColor : BorderColor);
    graphic.drawText({{textX, textY}, {1.0f, 1.0f}, 0.0f}, label, textSize, TextColor, 0, OrByte::RGBA{0, 0, 0, 0}, OrByte::constants::FONT_DEFAULT);
}

[[nodiscard]] std::string timestampedWorldName()
{
    const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

    return std::format("world_{:%Y-%m-%d_%H-%M-%S}{}", now, OrByte::WORLD_EXTENSION);
}

} // namespace

OrByte::FilesManagementWidget::FilesManagementWidget(World& world, const ComponentFactories& factories, int width)
    : Widget("", width), _world(world), _factories(factories), _baseWidth(width)
{
}

void OrByte::FilesManagementWidget::_compile()
{
    std::cout << "compile" << std::endl;
}

void OrByte::FilesManagementWidget::_save()
{
    const std::filesystem::path configs{constants::CONFIGS_DIR};
    const std::filesystem::path path = configs / timestampedWorldName();

    try {
        std::filesystem::create_directories(configs);
        serializeWorld(this->_world, path);
        std::cout << "World saved to " << path.string() << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "Cannot save world: " << error.what() << std::endl;
    }
    if (this->_picking)
        this->_refreshFiles();
}

void OrByte::FilesManagementWidget::_load(const std::filesystem::path& path)
{
    try {
        deserializeWorld(this->_world, path, this->_factories);
        std::cout << "World loaded from " << path.string() << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "Cannot load world: " << error.what() << std::endl;
    }
    this->_world.setEntitySelected(-1);
    this->_closePicker();
}

void OrByte::FilesManagementWidget::_refreshFiles()
{
    std::error_code error;

    this->_files.clear();
    for (const auto& entry : std::filesystem::directory_iterator(constants::CONFIGS_DIR, error)) {
        if (entry.is_regular_file(error) && entry.path().extension() == WORLD_EXTENSION)
            this->_files.push_back(entry.path());
    }
    // most recent saves first
    std::ranges::sort(this->_files, [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
        std::error_code ignored;
        return std::filesystem::last_write_time(lhs, ignored) > std::filesystem::last_write_time(rhs, ignored);
    });
    this->_scrollOffset = 0;

    const int needed = pickerX() + static_cast<int>(std::max<std::size_t>(this->_files.size(), 1)) * (FileButtonWidth + Padding);
    const int available = this->getParent() != nullptr ? this->getParent()->getRect().w : needed;
    this->setSize(std::max(this->_baseWidth, std::min(needed, available)));
}

void OrByte::FilesManagementWidget::_openPicker()
{
    this->_picking = true;
    this->_refreshFiles();
}

void OrByte::FilesManagementWidget::_closePicker()
{
    this->_picking = false;
    this->_files.clear();
    this->_scrollOffset = 0;
    this->setSize(this->_baseWidth);
}

void OrByte::FilesManagementWidget::_onClick(Vector2 localMouse)
{
    const auto x = static_cast<int>(localMouse.x);
    const auto y = static_cast<int>(localMouse.y);
    const int height = this->getContentRect().h;

    if (buttonRect(Button::Compile, height).contains(x, y)) {
        this->_compile();
        return;
    }
    if (buttonRect(Button::Save, height).contains(x, y)) {
        this->_save();
        return;
    }
    if (buttonRect(Button::Load, height).contains(x, y)) {
        if (this->_picking)
            this->_closePicker();
        else
            this->_openPicker();
        return;
    }
    if (!this->_picking)
        return;

    const std::size_t slots = visibleFiles(this->getContentRect().w);
    for (std::size_t slot = 0; slot < slots && this->_scrollOffset + slot < this->_files.size(); ++slot) {
        if (fileRect(slot, height).contains(x, y)) {
            this->_load(this->_files[this->_scrollOffset + slot]);
            return;
        }
    }
}

void OrByte::FilesManagementWidget::draw(const IGraphic& graphic, Vector2u size)
{
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);

    graphic.drawRectangle({{static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f}, {1.0f, 1.0f}, 0.0f},
        {static_cast<float>(width), static_cast<float>(height)}, RGBA{40, 40, 40, 255}, 1, BorderColor);
    for (std::size_t i = 0; i < ButtonLabels.size(); ++i) {
        const auto button = static_cast<Button>(i);
        drawButton(graphic, buttonRect(button, height), ButtonLabels.at(i), ButtonTextSize, button == Button::Load && this->_picking);
    }
    if (!this->_picking)
        return;

    const auto separatorX = static_cast<float>(pickerX() - Padding) - 0.5f;
    graphic.drawRectangle({{separatorX, static_cast<float>(height) * 0.5f}, {1.0f, 1.0f}, 0.0f},
        {1.0f, static_cast<float>(std::max(0, height - 2 * Padding))}, BorderColor, 0, RGBA{0, 0, 0});

    if (this->_files.empty()) {
        const std::string message = std::string("No world saved in ") + constants::CONFIGS_DIR;
        graphic.drawText({{static_cast<float>(pickerX()), (static_cast<float>(height) - FileTextSize) * 0.5f - 2.0f}, {1.0f, 1.0f}, 0.0f},
            message, FileTextSize, MutedText, 0, RGBA{0, 0, 0, 0}, constants::FONT_DEFAULT);
        return;
    }

    const std::size_t slots = visibleFiles(width);
    for (std::size_t slot = 0; slot < slots && this->_scrollOffset + slot < this->_files.size(); ++slot) {
        const Rect rect = fileRect(slot, height);
        const std::string name = this->_files[this->_scrollOffset + slot].filename().string();
        drawButton(graphic, rect, fitText(name, FileTextSize, rect.w - 8), FileTextSize, false);
    }
}

bool OrByte::FilesManagementWidget::onEvent(const Event& event, Vector2 localMouse)
{
    if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Left) {
        this->_onClick(localMouse);
        return true;
    }
    if (event.type == Event::Type::MouseWheelScrolled && this->_picking) {
        const std::size_t slots = visibleFiles(this->getContentRect().w);
        const std::size_t maxScroll = this->_files.size() > slots ? this->_files.size() - slots : 0;

        if (event.wheelDelta > 0.0 && this->_scrollOffset > 0)
            --this->_scrollOffset;
        else if (event.wheelDelta < 0.0 && this->_scrollOffset < maxScroll)
            ++this->_scrollOffset;
        return true;
    }
    if (event.type == Event::Type::KeyPressed && event.key == Event::Key::Escape && this->_picking) {
        this->_closePicker();
        return true;
    }
    return false;
}
