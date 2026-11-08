/*
** EPITECH PROJECT, 2026
** RType
** File description:
** FilesManagementWidget.cpp
*/

#include "GameEngine/Layout/FilesManagementWidget.hpp"
#include "GameEngine/Layout/AssetsWidget.hpp"
#include "constants.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <system_error>

namespace {

enum class Button : std::uint8_t {
    Compile,
    Save,
    Load,
    Project,
};

constexpr std::array<const char *, 4> ButtonLabels{"Compile", "Save", "Load", "Open Project"};
constexpr int ButtonCount = static_cast<int>(ButtonLabels.size());

constexpr int Padding = 6;
constexpr int ButtonWidth = 90;
constexpr int FileButtonWidth = 210;
constexpr int NameFieldWidth = 260;
constexpr std::size_t NameMaxLength = 64;
constexpr unsigned ButtonTextSize = 14;
constexpr unsigned FileTextSize = 12;
constexpr float GlyphWidthRatio = 0.55F;
constexpr float TextCenterRatio = 0.62F;
constexpr const char *SaveHint = "Enter: save   Esc: cancel";
constexpr const char *OverwriteHint = "Enter: overwrite   Esc: cancel";

const OrByte::RGBA ButtonColor(60, 60, 60, 255);
const OrByte::RGBA ActiveColor(52, 62, 74, 255);
const OrByte::RGBA FieldColor(26, 31, 38, 255);
const OrByte::RGBA BorderColor(80, 80, 80, 255);
const OrByte::RGBA ActiveBorderColor(120, 160, 200, 255);
const OrByte::RGBA TextColor(220, 220, 220, 255);
const OrByte::RGBA MutedText(170, 180, 196, 255);
const OrByte::RGBA WarningText(220, 170, 90, 255);

[[nodiscard]] Rect buttonRect(Button button, int height)
{
    const int index = static_cast<int>(button);

    return {Padding + (index * (ButtonWidth + Padding)), Padding, ButtonWidth, std::max(0, height - (2 * Padding))};
}

[[nodiscard]] constexpr int pickerX()
{
    return Padding + (ButtonCount * (ButtonWidth + Padding)) + Padding;
}

[[nodiscard]] Rect fileRect(std::size_t slot, int height)
{
    return {pickerX() + (static_cast<int>(slot) * (FileButtonWidth + Padding)), Padding, FileButtonWidth, std::max(0, height - (2 * Padding))};
}

[[nodiscard]] Rect nameRect(int height)
{
    return {pickerX(), Padding, NameFieldWidth, std::max(0, height - (2 * Padding))};
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

[[nodiscard]] float textTop(const Rect& rect, unsigned size)
{
    return static_cast<float>(rect.y) + (static_cast<float>(rect.h) * 0.5F) - (static_cast<float>(size) * TextCenterRatio);
}

[[nodiscard]] int namingWidth()
{
    return pickerX() + NameFieldWidth + Padding + static_cast<int>(textWidth(OverwriteHint, FileTextSize)) + Padding;
}

[[nodiscard]] std::size_t fittingChars(unsigned size, int width)
{
    return static_cast<std::size_t>(static_cast<float>(width) / (static_cast<float>(size) * GlyphWidthRatio));
}

[[nodiscard]] std::string fitText(const std::string& text, unsigned size, int width)
{
    const std::size_t maxChars = fittingChars(size, width);

    if (text.size() <= maxChars || maxChars < 3)
        return text;
    return text.substr(0, maxChars - 3) + "...";
}

[[nodiscard]] std::string fitTail(const std::string& text, unsigned size, int width)
{
    const std::size_t maxChars = fittingChars(size, width);

    if (text.size() <= maxChars || maxChars < 3)
        return text;
    return "..." + text.substr(text.size() - (maxChars - 3));
}

[[nodiscard]] bool isNameChar(unsigned character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9') || character == '_' || character == '-';
}

[[nodiscard]] std::filesystem::path worldPath(const std::string& name)
{
    return std::filesystem::path(OrByte::constants::CONFIGS_DIR) / (name + OrByte::WORLD_EXTENSION);
}

void drawBox(const OrByte::IGraphic& graphic, const Rect& rect, const OrByte::RGBA& fill, const OrByte::RGBA& border)
{
    const auto w = static_cast<float>(rect.w);
    const auto h = static_cast<float>(rect.h);

    graphic.drawRectangle({{static_cast<float>(rect.x) + (w * 0.5F), static_cast<float>(rect.y) + (h * 0.5F)}, {1.0F, 1.0F}, 0.0F},
        {w, h}, fill, 1, border);
}

void drawLabel(const OrByte::IGraphic& graphic, float x, float y, const std::string& text, unsigned size, const OrByte::RGBA& color)
{
    graphic.drawText({{x, y}, {1.0F, 1.0F}, 0.0F}, text, size, 0.f, color, 0, OrByte::RGBA(0, 0, 0, 0), OrByte::constants::FONT_DEFAULT);
}

void drawButton(const OrByte::IGraphic& graphic, const Rect& rect, const std::string& label, unsigned textSize, bool active)
{
    const float textX = static_cast<float>(rect.x) + std::max(4.0F, (static_cast<float>(rect.w) - textWidth(label, textSize)) * 0.5F);

    drawBox(graphic, rect, active ? ActiveColor : ButtonColor, active ? ActiveBorderColor : BorderColor);
    drawLabel(graphic, textX, textTop(rect, textSize), label, textSize, TextColor);
}

[[nodiscard]] std::string timestampedWorldName()
{
    const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

    return std::format("world_{:%Y-%m-%d_%H-%M-%S}", now);
}

} // namespace

OrByte::FilesManagementWidget::FilesManagementWidget(World& world, const ComponentFactories& factories, AssetsWidget& assets, int width)
    : Widget("", width), _world(world), _factories(factories), _assets(assets), _baseWidth(width)
{
}

void OrByte::FilesManagementWidget::_compile()
{
    try {
        serializeWorld(this->_world, constants::GAME_WORLD);
    } catch (const std::exception& error) {
        std::cerr << "Cannot export world: " << error.what() << std::endl;
        return;
    }
    this->_world.ensureCamera();
    system("cmake -S cmake/GameEngine -B build-solo && cmake --build build-solo");
}

void OrByte::FilesManagementWidget::_save()
{
    const std::filesystem::path path = worldPath(this->_fileName.empty() ? this->_defaultName : this->_fileName);

    try {
        std::filesystem::create_directories(path.parent_path());
        serializeWorld(this->_world, path);
        std::cout << "World saved to " << path.string() << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "Cannot save world: " << error.what() << std::endl;
        return;
    }
    this->_close();
}

void OrByte::FilesManagementWidget::_load(const std::filesystem::path& path)
{
    try {
        deserializeWorld(this->_world, path, this->_factories);
        std::cout << "World loaded from " << path.string() << std::endl;
    } catch (const std::exception& error) {
        std::cerr << "Cannot load world: " << error.what() << std::endl;
    }
    this->_world.ensureCamera();
    this->_world.clearSelectedEntity();
    this->_close();
}

void OrByte::FilesManagementWidget::_refreshFiles()
{
    std::error_code error;

    this->_files.clear();
    for (const auto& entry : std::filesystem::directory_iterator(constants::CONFIGS_DIR, error)) {
        if (entry.is_regular_file(error) && entry.path().extension() == WORLD_EXTENSION)
            this->_files.push_back(entry.path());
    }
    std::ranges::sort(this->_files, [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
        std::error_code ignored;
        return std::filesystem::last_write_time(lhs, ignored) > std::filesystem::last_write_time(rhs, ignored);
    });
    this->_scrollOffset = 0;
    this->_resize(pickerX() + (static_cast<int>(std::max<std::size_t>(this->_files.size(), 1)) * (FileButtonWidth + Padding)));
}

void OrByte::FilesManagementWidget::_resize(int needed)
{
    const int available = this->getParent() != nullptr ? this->getParent()->getRect().w : needed;

    this->setSize(std::max(this->_baseWidth, std::min(needed, available)));
}

void OrByte::FilesManagementWidget::_openProjectPicker(const std::filesystem::path& path)
{
    try {
        const auto directory = std::filesystem::canonical(path);
        std::vector<std::filesystem::path> directories{directory, directory.parent_path()};
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            std::error_code error;
            if (entry.is_directory(error))
                directories.push_back(entry.path());
        }
        std::sort(directories.begin() + 2, directories.end());
        this->_files = std::move(directories);
        this->_projectPath = directory;
        this->_mode = Mode::PickingProject;
        this->_scrollOffset = 0;
        this->_resize(pickerX() + static_cast<int>(this->_files.size()) * (FileButtonWidth + Padding));
    } catch (const std::exception& error) {
        std::cerr << "Cannot open project directory: " << error.what() << std::endl;
    }
}

void OrByte::FilesManagementWidget::_openPicker()
{
    this->_mode = Mode::Picking;
    this->_fileName.clear();
    this->_refreshFiles();
}

void OrByte::FilesManagementWidget::_openNaming()
{
    this->_mode = Mode::Naming;
    this->_files.clear();
    this->_scrollOffset = 0;
    this->_fileName.clear();
    this->_defaultName = timestampedWorldName();
    this->_resize(namingWidth());
}

void OrByte::FilesManagementWidget::_close()
{
    this->_mode = Mode::Idle;
    this->_files.clear();
    this->_scrollOffset = 0;
    this->_fileName.clear();
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
        if (this->_mode == Mode::Naming)
            this->_save();
        else
            this->_openNaming();
        return;
    }
    if (buttonRect(Button::Load, height).contains(x, y)) {
        if (this->_mode == Mode::Picking)
            this->_close();
        else
            this->_openPicker();
        return;
    }
    if (buttonRect(Button::Project, height).contains(x, y)) {
        if (this->_mode == Mode::PickingProject)
            this->_close();
        else
            this->_openProjectPicker(this->_assets.getDirPath());
        return;
    }
    if (this->_mode != Mode::Picking && this->_mode != Mode::PickingProject)
        return;

    const std::size_t slots = visibleFiles(this->getContentRect().w);
    for (std::size_t slot = 0; slot < slots && this->_scrollOffset + slot < this->_files.size(); ++slot) {
        if (fileRect(slot, height).contains(x, y)) {
            const std::size_t index = this->_scrollOffset + slot;
            if (this->_mode == Mode::PickingProject) {
                if (index == 0) {
                    try {
                        this->_assets.setDirPath(this->_projectPath);
                        this->_close();
                    } catch (const std::exception& error) {
                        std::cerr << "Cannot select project directory: " << error.what() << std::endl;
                    }
                } else {
                    this->_openProjectPicker(this->_files[index]);
                }
            } else {
                this->_load(this->_files[index]);
            }
            return;
        }
    }
}

bool OrByte::FilesManagementWidget::_onNamingEvent(const Event& event)
{
    if (event.type == Event::Type::TextEntered) {
        if (isNameChar(event.keyChar) && this->_fileName.size() < NameMaxLength)
            this->_fileName.push_back(static_cast<char>(event.keyChar));
        return true;
    }
    if (event.type != Event::Type::KeyPressed)
        return false;
    if (event.key == Event::Key::Enter)
        this->_save();
    else if (event.key == Event::Key::Escape)
        this->_close();
    else if (event.key == Event::Key::Backspace && !this->_fileName.empty())
        this->_fileName.pop_back();
    return true;
}

void OrByte::FilesManagementWidget::_drawPicker(const IGraphic& graphic, int width, int height) const
{
    if (this->_files.empty()) {
        const std::string message = std::string("No world saved in ") + constants::CONFIGS_DIR;
        drawLabel(graphic, static_cast<float>(pickerX()), textTop(fileRect(0, height), FileTextSize), message, FileTextSize, MutedText);
        return;
    }

    const std::size_t slots = visibleFiles(width);
    for (std::size_t slot = 0; slot < slots && this->_scrollOffset + slot < this->_files.size(); ++slot) {
        const Rect rect = fileRect(slot, height);
        const std::size_t index = this->_scrollOffset + slot;
        std::string name = this->_files[index].filename().string();
        if (this->_mode == Mode::PickingProject) {
            if (index == 0)
                name = "Open here: " + this->_projectPath.string();
            else if (index == 1)
                name = ".. (parent)";
            else
                name += "/";
        }
        drawButton(graphic, rect, fitText(name, FileTextSize, rect.w - 8), FileTextSize, false);
    }
}

void OrByte::FilesManagementWidget::_drawNaming(const IGraphic& graphic, int height) const
{
    const Rect field = nameRect(height);
    const float textY = textTop(field, FileTextSize);
    const auto extensionWidth = static_cast<int>(textWidth(WORLD_EXTENSION, FileTextSize));
    const bool placeholder = this->_fileName.empty();
    const std::string text = placeholder ? "|" + this->_defaultName : this->_fileName + "|";
    std::error_code error;
    const bool overwrite = std::filesystem::exists(worldPath(placeholder ? this->_defaultName : this->_fileName), error);

    drawBox(graphic, field, FieldColor, ActiveBorderColor);
    drawLabel(graphic, static_cast<float>(field.x + Padding), textY, fitTail(text, FileTextSize, field.w - (3 * Padding) - extensionWidth),
        FileTextSize, placeholder ? MutedText : TextColor);
    drawLabel(graphic, static_cast<float>(field.x + field.w - Padding - extensionWidth), textY, WORLD_EXTENSION, FileTextSize, MutedText);
    drawLabel(graphic, static_cast<float>(field.x + field.w + Padding), textY, overwrite ? OverwriteHint : SaveHint, FileTextSize,
        overwrite ? WarningText : MutedText);
}

void OrByte::FilesManagementWidget::draw(const IGraphic& graphic, Vector2u size)
{
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);

    drawBox(graphic, Rect(0, 0, width, height), RGBA(40, 40, 40, 255), BorderColor);
    for (std::size_t i = 0; i < ButtonLabels.size(); ++i) {
        const auto button = static_cast<Button>(i);
        const bool active = (button == Button::Save && this->_mode == Mode::Naming)
            || (button == Button::Load && this->_mode == Mode::Picking)
            || (button == Button::Project && this->_mode == Mode::PickingProject);
        drawButton(graphic, buttonRect(button, height), ButtonLabels.at(i), ButtonTextSize, active);
    }
    if (this->_mode == Mode::Idle)
        return;

    const auto separatorX = static_cast<float>(pickerX() - Padding) - 0.5F;
    graphic.drawRectangle({{separatorX, static_cast<float>(height) * 0.5F}, {1.0F, 1.0F}, 0.0F},
        {1.0F, static_cast<float>(std::max(0, height - (2 * Padding)))}, BorderColor, 0, RGBA(0, 0, 0));
    if (this->_mode == Mode::Naming)
        this->_drawNaming(graphic, height);
    else
        this->_drawPicker(graphic, width, height);
}

bool OrByte::FilesManagementWidget::onEvent(const Event& event, Vector2 localMouse)
{
    if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Left) {
        this->_onClick(localMouse);
        return true;
    }
    if (this->_mode == Mode::Naming)
        return this->_onNamingEvent(event);
    if (event.type == Event::Type::MouseWheelScrolled && (this->_mode == Mode::Picking || this->_mode == Mode::PickingProject)) {
        const std::size_t slots = visibleFiles(this->getContentRect().w);
        const std::size_t maxScroll = this->_files.size() > slots ? this->_files.size() - slots : 0;

        if (event.wheelDelta > 0.0 && this->_scrollOffset > 0)
            --this->_scrollOffset;
        else if (event.wheelDelta < 0.0 && this->_scrollOffset < maxScroll)
            ++this->_scrollOffset;
        return true;
    }
    if (event.type == Event::Type::KeyPressed && event.key == Event::Key::Escape
        && (this->_mode == Mode::Picking || this->_mode == Mode::PickingProject)) {
        this->_close();
        return true;
    }
    return false;
}
