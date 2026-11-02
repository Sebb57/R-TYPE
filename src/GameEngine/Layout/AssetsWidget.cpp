/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AssetsWidget.cpp
*/

#include "GameEngine/Layout/AssetsWidget.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sys/stat.h>

namespace {

inline const std::filesystem::path ICON_PATH = "assets/file.png";
inline const std::filesystem::path HICON_PATH = "assets/directory.png";

constexpr float TITLE_HEIGHT = 24.f;
constexpr float MINIMUM_ROW_HEIGHT = 32.f;
constexpr float TEXT_GAP = 4.f;
constexpr float TEXT_SIZE = 12.f;
constexpr float ENTRY_OFFSET_X = 16.f;
constexpr float ENTRY_OFFSET_Y = 16.f;
constexpr float ENTRY_GAP_X = 16.f;
constexpr float ENTRY_GAP_Y = 16.f + TEXT_SIZE;
constexpr float ENTRY_SIZE = 80.f;
constexpr float SCROLL_SENSITIVITY = 10.f;

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
    const float headerHeight = std::min(height, TITLE_HEIGHT);
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
    std::cout << path.c_str() << std::endl;
    return false;
}

} // namespace

OrByte::AssetsWidget::AssetsWidget(World& world, int width) : Widget("Assets", width), _world(world)
{
    const char* pwd = std::getenv("PWD");
    if (!pwd)
        throw std::runtime_error("'PWD' isn't set in the environment");
    this->_dirPath = pwd;

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

    const RGBA bgColor = RGBA{40, 40, 40, 255};
    const RGBA headerColor = RGBA{60, 60, 60, 255};
    const RGBA borderColor = RGBA{80, 80, 80, 255};
    const RGBA textColor = RGBA{220, 220, 220, 255};

    const float centerX = static_cast<float>(width) * 0.5f;
    const float centerY = static_cast<float>(height) * 0.5f;
    const float headerCenterY = static_cast<float>(metrics.headerHeight) * 0.5f;

    // Background
    graphic.drawRectangle({{centerX, centerY}, {1.f, 1.f}, 0.f},
        {static_cast<float>(width), static_cast<float>(height)}, bgColor, 1, borderColor);

    // Entries
    const int effectiveWidth = this->getRect().w - ENTRY_OFFSET_X * 2;
    const int effectiveHeight = this->getRect().h - ENTRY_OFFSET_Y * 2 - TITLE_HEIGHT;
    const int entriesPerRow = std::max(1, 1 + static_cast<int>((effectiveWidth - ENTRY_SIZE) / (ENTRY_SIZE + ENTRY_GAP_X)));
    const int nRow = this->_entries.size() / entriesPerRow;
    const float minOffset = (ENTRY_SIZE * (nRow + 1) + ENTRY_GAP_Y * (nRow + 0.5) - effectiveHeight) * -1;

    this->_scrollOffset = std::min(0.f, std::max(minOffset, this->_scrollOffset));
    for (std::size_t i = 0; i < this->_entries.size(); i++) {
        const int row = i / entriesPerRow;
        const int col = i % entriesPerRow;
        const float centerX = ENTRY_OFFSET_X + (col + 0.5) * ENTRY_SIZE + col * ENTRY_GAP_X;
        const float centerY = ENTRY_OFFSET_Y + (row + 0.5) * ENTRY_SIZE + row * ENTRY_GAP_Y + TITLE_HEIGHT + this->_scrollOffset;

        if (isFileImage(this->_entries[i]))
            graphic.drawImage({{centerX, centerY}}, {ENTRY_SIZE, ENTRY_SIZE}, this->_entries[i]);
        else if (isFileDir(this->_dirPath / this->_entries[i]))
            graphic.drawImage({{centerX, centerY}}, {ENTRY_SIZE, ENTRY_SIZE}, HICON_PATH);
        else
            graphic.drawImage({{centerX, centerY}}, {ENTRY_SIZE, ENTRY_SIZE}, ICON_PATH);
        graphic.drawText({{centerX - ENTRY_SIZE / 2, centerY + TEXT_GAP + ENTRY_SIZE / 2}}, this->_entries[i], TEXT_SIZE, ENTRY_SIZE);
    }

    // Header
    graphic.drawRectangle({{centerX, headerCenterY}, {1.f, 1.f}, 0.f},
        {static_cast<float>(width), static_cast<float>(metrics.headerHeight)}, headerColor, 0, RGBA{0, 0, 0});
    graphic.drawText({{6.f, 3.f}, {1.f, 1.f}, 0.f}, this->getTitle(), 14, 0.f, textColor, 0, RGBA{0, 0, 0, 0}, constants::FONT_DEFAULT);
}

bool OrByte::AssetsWidget::onEvent(const Event& event, Vector2 localMouse)
{
    switch (event.type) {
        case Event::Type::MouseWheelScrolled:
            if (localMouse.y > TITLE_HEIGHT) {
                this->_scrollOffset += event.wheelDelta * SCROLL_SENSITIVITY;
                return true;
            }
            return false;
        default:
            break;
    }

    return false;
}
