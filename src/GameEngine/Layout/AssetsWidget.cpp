/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AssetsWidget.cpp
*/

#include "GameEngine/Layout/AssetsWidget.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sys/stat.h>

namespace {

inline const std::filesystem::path ICON_PATH = "assets/file.png";
inline const std::filesystem::path HICON_PATH = "assets/directory.png";

constexpr OrByte::RGBA bgColor {40, 40, 40, 255};
constexpr OrByte::RGBA headerColor {60, 60, 60, 255};
constexpr OrByte::RGBA borderColor {80, 80, 80, 255};
constexpr OrByte::RGBA textColor {220, 220, 220, 255};
constexpr OrByte::RGBA hoverColor {124, 136, 148};
constexpr OrByte::RGBA selectedColor {88, 100, 112};
constexpr OrByte::RGBA dirsColor {80, 80, 80, 255};

constexpr float TITLE_HEIGHT = 24.f;
constexpr float DIRS_HEIGHT = 24.f;
constexpr float HEADER_HEIGHT = TITLE_HEIGHT + DIRS_HEIGHT;
constexpr float MINIMUM_ROW_HEIGHT = 32.f;
constexpr float TEXT_GAP = 4.f;
constexpr float TEXT_SIZE = 12.f;
constexpr float DIRS_OFFSET_X = 8.f;
constexpr float ENTRY_OFFSET_X = 16.f;
constexpr float ENTRY_OFFSET_Y = 16.f;
constexpr float ENTRY_GAP_X = 16.f;
constexpr float ENTRY_GAP_Y = 16.f + TEXT_SIZE;
constexpr float DIRS_GAP = 4.f;
constexpr float DIRS_MARGIN = 4.f;
constexpr float ENTRY_SIZE = 80.f;
constexpr float SCROLL_SENSITIVITY = 10.f;

constexpr float DOUBLE_CLICK_GAP = 0.5f;

struct AssetsLayout {
    float headerHeight;
    float rowHeight;
    float padding;
    float idColumnWidth;
    float rowTextSize;
    std::size_t visibleRows;
};

[[nodiscard]] AssetsLayout makeLayout(const Rect& bounds)
{
    const float width = std::max(1.f, static_cast<float>(bounds.w));
    const float height = std::max(1.f, static_cast<float>(bounds.h));
    const float headerHeight = std::min(height, HEADER_HEIGHT);
    const float rowsHeight = height - headerHeight;
    const auto visibleRows = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(rowsHeight / MINIMUM_ROW_HEIGHT)));
    const float rowHeight = std::max(1.f, rowsHeight / static_cast<float>(visibleRows));
    const float padding = std::max(8.f, width * 0.04f);
    const float idColumnWidth = std::max(34.f, width * 0.18f);
    const float rowTextSize = std::clamp(rowHeight * 0.5f, 11.f, 14.f);

    return {headerHeight, rowHeight, padding, idColumnWidth, rowTextSize, visibleRows};
}

std::string toLowerString(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

std::size_t getFileSize(const std::filesystem::path path)
{
    if (!std::filesystem::exists(path))
        return 0;

    if (std::filesystem::is_regular_file(path))
        return static_cast<std::size_t>(std::filesystem::file_size(path));

    std::size_t size = 0;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
        if (std::filesystem::is_regular_file(entry.path()))
            size += std::filesystem::file_size(entry.path());

    return size;
}

bool isFileImage(const std::filesystem::path file)
{
    if (file.extension() == ".png" ||
        file.extension() == ".jpg"||
        file.extension() == ".jpeg"||
        file.extension() == ".webp")
        return true;
    return false;
}

bool isFileDir(const std::filesystem::path path)
{
    struct stat s;

    if (stat(path.c_str(), &s) == 0)
        if (s.st_mode & S_IFDIR)
            return true;
    return false;
}

int getEntriesPerRow(const int width)
{
    const int effectiveWidth = width - ENTRY_OFFSET_X * 2;

    return std::max(1, 1 + static_cast<int>((effectiveWidth - ENTRY_SIZE) / (ENTRY_SIZE + ENTRY_GAP_X)));
}

Vector2 getEntryPos(const int i, const int entriesPerRow, const float scrollOffset)
{
    const int row = i / entriesPerRow;
    const int col = i % entriesPerRow;
    const float centerX = ENTRY_OFFSET_X + (col + 0.5) * ENTRY_SIZE + col * ENTRY_GAP_X;
    const float centerY = ENTRY_OFFSET_Y + (row + 0.5) * ENTRY_SIZE + row * ENTRY_GAP_Y + HEADER_HEIGHT + scrollOffset;

    return {centerX, centerY};
}

Vector2 drawButton(const OrByte::IGraphic& graphic, Vector2 topLeft, const std::string string)
{
    const Vector2 textSize = graphic.getTextSize(string, TEXT_SIZE);

    graphic.drawRectangle({{topLeft.x + textSize.x / 2 + DIRS_MARGIN, topLeft.y + textSize.y / 2 + DIRS_MARGIN}},
        {textSize.x + DIRS_MARGIN*2, textSize.y + DIRS_MARGIN*2}, dirsColor, 1, borderColor);
    graphic.drawText({{topLeft.x + DIRS_MARGIN, topLeft.y + DIRS_MARGIN}}, string, TEXT_SIZE, 0.f, {255, 255, 255});
    return { textSize.x + DIRS_MARGIN*2, textSize.y + DIRS_MARGIN*2 };
}

} // namespace


OrByte::AssetsWidget::AssetsWidget(World& world, int width) : Widget("Assets", width), _world(world)
{
    const char* pwd = std::getenv("PWD");
    if (!pwd)
        throw std::runtime_error("'PWD' isn't set in the environment");
    this->_dirPath = pwd;
    this->_startPath = this->_dirPath;

    this->_chronoClick.reset();
    this->_getEntries();
}

void OrByte::AssetsWidget::_getEntries()
{
    struct dirent* entry;

    // Get Entries
    this->_dir = opendir(this->_dirPath.c_str());
    if (!this->_dir)
        throw std::runtime_error("Couldn't open directory");
    this->_entries.clear();
    while ((entry = readdir(this->_dir)) != nullptr) {
        std::string entryName = std::string(entry->d_name);
        if (entryName == "." || entryName == "..")
            continue;
        if (entryName[0] == '.' && !this->_showHidden)
            continue;
        this->_entries.push_back(entryName);
    }
    this->_sortEntries(this->_sortType, this->_ascending);
}

void OrByte::AssetsWidget::_sortEntries(SortType type, bool ascending)
{
    switch (type) {
        case SortType::Name:
            std::sort(this->_entries.begin(), this->_entries.end(),
                [ascending](const auto& a, const auto& b) {
                    std::string aName = toLowerString(a);
                    std::string bName = toLowerString(b);

                    return ascending ? (aName < bName) : (aName > bName);
                });
            break;
        case SortType::Size:
            std::sort(this->_entries.begin(), this->_entries.end(),
                [ascending](const auto& a, const auto& b) {
                    std::size_t aSize = getFileSize(a);
                    std::size_t bSize = getFileSize(b);

                    return ascending ? (aSize < bSize) : (aSize > bSize);
                });
            break;
    }
}

void OrByte::AssetsWidget::draw(const IGraphic& graphic, Vector2u size)
{
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const auto metrics = makeLayout(Rect(0, 0, width, height));

    const float centerX = static_cast<float>(width) * 0.5f;
    const float centerY = static_cast<float>(height) * 0.5f;
    const float headerCenterY = static_cast<float>(metrics.headerHeight) * 0.5f;

    // Background
    graphic.drawRectangle({{centerX, centerY}, {1.f, 1.f}, 0.f},
        {static_cast<float>(width), static_cast<float>(height)}, bgColor, 1, borderColor);

    // Entries
    const Rect rect = this->getRect();
    const int entriesPerRow = getEntriesPerRow(rect.w);
    const int effectiveHeight = this->getRect().h - ENTRY_OFFSET_Y * 2 - HEADER_HEIGHT;
    const int nRow = this->_entries.size() / entriesPerRow;
    const float minOffset = (ENTRY_SIZE * (nRow + 1) + ENTRY_GAP_Y * (nRow + 0.5) - effectiveHeight) * -1;

    this->_scrollOffset = std::min(0.f, std::max(minOffset, this->_scrollOffset));
    for (std::size_t i = 0; i < this->_entries.size(); i++) {
        const Vector2 center = getEntryPos(i, entriesPerRow, this->_scrollOffset);

        if (this->_selected == i)
            graphic.drawRectangle({{center.x, center.y}}, {ENTRY_SIZE, ENTRY_SIZE}, selectedColor);
        else if (this->_hovered == i)
            graphic.drawRectangle({{center.x, center.y}}, {ENTRY_SIZE, ENTRY_SIZE}, hoverColor);

        if (isFileImage(this->_entries[i]))
            graphic.drawImage({{center.x, center.y}}, {ENTRY_SIZE, ENTRY_SIZE}, this->_dirPath / this->_entries[i], true);
        else if (isFileDir(this->_dirPath / this->_entries[i]))
            graphic.drawImage({{center.x, center.y}}, {ENTRY_SIZE, ENTRY_SIZE}, HICON_PATH);
        else
            graphic.drawImage({{center.x, center.y}}, {ENTRY_SIZE, ENTRY_SIZE}, ICON_PATH);
        graphic.drawText({{center.x - ENTRY_SIZE / 2, center.y + TEXT_GAP + ENTRY_SIZE / 2}}, this->_entries[i], TEXT_SIZE, ENTRY_SIZE);
    }

    // Header
    graphic.drawRectangle({{centerX, headerCenterY}, {1.f, 1.f}, 0.f},
        {static_cast<float>(width), static_cast<float>(metrics.headerHeight)}, headerColor, 0, RGBA{0, 0, 0});
    graphic.drawRectangle({{centerX, TITLE_HEIGHT * 1.5}, {1.f, 1.f}, 0.f},
        {static_cast<float>(width), TITLE_HEIGHT}, headerColor, 2, borderColor);
    graphic.drawText({{6.f, 3.f}, {1.f, 1.f}, 0.f}, this->getTitle(), 14, 0.f, textColor, 0, RGBA{0, 0, 0, 0}, constants::FONT_DEFAULT);


    // Dirs
    std::filesystem::path relative = this->_dirPath.lexically_relative(this->_startPath);
    std::filesystem::path current = this->_startPath;
    float currX = DIRS_OFFSET_X;

    currX += drawButton(graphic, {currX, TITLE_HEIGHT + 2.f}, current.filename()).x + DIRS_GAP;
    for (const auto& component: relative) {
        if (component.filename() == ".")
            break;

        current /= component;

        currX += graphic.drawText({{currX, TITLE_HEIGHT + 2.f}}, ">", TEXT_SIZE*1.45, 0.f, {255, 255, 255}).x + DIRS_GAP;
        currX += drawButton(graphic, {currX, TITLE_HEIGHT + 2.f}, current.filename()).x + DIRS_GAP;
    }
}

bool OrByte::AssetsWidget::onEvent(const Event& event, Vector2 localMouse)
{
    const int entriesPerRow = getEntriesPerRow(this->getRect().w);

    switch (event.type) {
        case Event::Type::MouseWheelScrolled:
            if (localMouse.y > HEADER_HEIGHT) {
                this->_scrollOffset += event.wheelDelta * SCROLL_SENSITIVITY;
                return true;
            }
            return false;
        case Event::Type::MouseButtonPressed:
            for (std::size_t i = 0; i < this->_entries.size(); i++) {
                const Vector2 center = getEntryPos(i, entriesPerRow, this->_scrollOffset);

                if (localMouse.x >= center.x - ENTRY_SIZE / 2 && localMouse.x < center.x + ENTRY_SIZE / 2 &&
                    localMouse.y >= center.y - ENTRY_SIZE / 2 && localMouse.y < center.y + ENTRY_SIZE / 2) {
                    this->_selected = i;
                    if (this->_selected == i && this->_chronoClick.getElapsedTime() <= DOUBLE_CLICK_GAP) {
                        if (isFileDir(this->_dirPath / this->_entries[i])) {
                            this->_dirPath = this->_dirPath / this->_entries[i];
                            this->_selected = -1;
                            this->_getEntries();
                        }
                    }
                    this->_chronoClick.reset();
                    return true;
                }
            }
            this->_selected = -1;
            return false;
        case Event::Type::MouseMoved:
            for (std::size_t i = 0; i < this->_entries.size(); i++) {
                const Vector2 center = getEntryPos(i, entriesPerRow, this->_scrollOffset);

                if (localMouse.x >= center.x - ENTRY_SIZE / 2 && localMouse.x < center.x + ENTRY_SIZE / 2 &&
                    localMouse.y >= center.y - ENTRY_SIZE / 2 && localMouse.y < center.y + ENTRY_SIZE / 2) {
                    this->_hovered = i;
                    return true;
                }
            }
            this->_hovered = -1;
            return false;
        default:
            break;
    }

    return false;
}
