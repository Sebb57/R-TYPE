/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Detail.cpp
*/

#include "GameEngine/Layout/Detail.hpp"
#include "GameEngine/ECS/Components/Script.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/Components/Camera.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Name.hpp"
#include "GameEngine/ECS/Components/Orbit.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/Poison.hpp"
#include "GameEngine/ECS/Components/RegenHealth.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Sinus.hpp"
#include "GameEngine/ECS/Components/Spiral.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Wither.hpp"
#include "GameEngine/ECS/Components/Music.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <type_traits>
#include <utility>

namespace OrByte {
namespace {

constexpr RGBA BG_COLOR{40, 40, 40, 255};
constexpr RGBA HEADER_COLOR{60, 60, 60, 255};
constexpr RGBA SECTION_COLOR{52, 62, 74, 255};
constexpr RGBA CELL_COLOR{26, 31, 38, 255};
constexpr RGBA READONLY_COLOR{34, 38, 44, 255};
constexpr RGBA BORDER_COLOR{80, 80, 80, 255};
constexpr RGBA EDIT_BORDER_COLOR{120, 160, 200, 255};
constexpr RGBA SELECTION_COLOR{60, 95, 140, 255};
constexpr RGBA BUTTON_COLOR{45, 85, 130, 255};
constexpr RGBA REMOVE_COLOR{120, 50, 50, 255};
constexpr RGBA TEXT_COLOR{220, 220, 220, 255};
constexpr RGBA MUTED_COLOR{170, 180, 196, 255};
constexpr RGBA NO_COLOR{0, 0, 0, 0};

constexpr unsigned SECTION_FONT = 14;
constexpr unsigned FIELD_FONT = 13;
constexpr float GLYPH_RATIO = 0.55f;

void fillRect(const IGraphic& graphic, float x, float y, float w, float h, RGBA fill, unsigned outline = 0, RGBA outlineColor = NO_COLOR)
{
    graphic.drawRectangle(Transform{ { x + w / 2.f, y + h / 2.f }, { 1.f, 1.f }, 0.f }, { w, h }, fill, outline, outlineColor);
}

void drawLabel(const IGraphic& graphic, float x, float y, const std::string& text, unsigned size, RGBA color)
{
    graphic.drawText(Transform{ { x, y }, { 1.f, 1.f }, 0.f }, text, size, 0.f, color, 0, NO_COLOR, constants::FONT_DEFAULT);
}

std::string fit(const std::string& text, float width, unsigned fontSize, bool keepEnd = false)
{
    const auto maxChars = static_cast<std::size_t>(std::max(0.f, width / (static_cast<float>(fontSize) * GLYPH_RATIO)));

    if (text.size() <= maxChars)
        return text;
    if (maxChars <= 2)
        return text.substr(keepEnd ? text.size() - maxChars : 0, maxChars);
    if (keepEnd)
        return ".." + text.substr(text.size() - (maxChars - 2));
    return text.substr(0, maxChars - 2) + "..";
}

RGBA tagColor(const std::string& tag)
{
    if (tag == "X" || tag == "R")
        return RGBA{220, 95, 95, 255};
    if (tag == "Y" || tag == "G")
        return RGBA{110, 200, 110, 255};
    if (tag == "B")
        return RGBA{100, 150, 230, 255};
    return MUTED_COLOR;
}

std::string trim(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t");
    if (first == std::string::npos)
        return "";
    const auto last = text.find_last_not_of(" \t");
    return text.substr(first, last - first + 1);
}

template <typename T>
std::string formatNumber(T value)
{
    if constexpr (std::is_floating_point_v<T>) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%g", static_cast<double>(value));
        return buffer;
    } else if constexpr (std::is_signed_v<T>) {
        return std::to_string(static_cast<long long>(value));
    } else {
        return std::to_string(static_cast<unsigned long long>(value));
    }
}

template <typename T>
bool parseNumber(const std::string& input, T& value)
{
    std::string text = trim(input);
    std::replace(text.begin(), text.end(), ',', '.');
    if (text.empty())
        return false;

    char *end = nullptr;
    if constexpr (std::is_floating_point_v<T>) {
        const double parsed = std::strtod(text.c_str(), &end);
        if (*end != '\0' || !std::isfinite(parsed))
            return false;
        value = static_cast<T>(parsed);
    } else {
        const long long parsed = std::strtoll(text.c_str(), &end, 10);
        if (*end != '\0')
            return false;
        if (parsed < static_cast<long long>(std::numeric_limits<T>::min())
            || static_cast<unsigned long long>(std::max(parsed, 0LL)) > static_cast<unsigned long long>(std::numeric_limits<T>::max()))
            return false;
        value = static_cast<T>(parsed);
    }
    return true;
}

const std::vector<std::string>& keyNames()
{
    static const std::vector<std::string> names = [] {
        std::vector<std::string> list = {
            "Unknown", "Escape", "Space", "Backspace", "Shift", "Control", "Alt",
            "Enter", "Left", "Right", "Up", "Down", "Delete", "Insert",
        };
        for (char letter = 'A'; letter <= 'Z'; ++letter)
            list.emplace_back(1, letter);
        return list;
    }();
    return names;
}

bool sameText(const std::string& a, const std::string& b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
        return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y));
    });
}

template <typename T>
Detail::Cell numberCell(T& value, std::string tag = "")
{
    Detail::Cell cell;
    cell.tag = std::move(tag);
    cell.get = [&value]() { return formatNumber(value); };
    cell.set = [&value](const std::string& text) { return parseNumber(text, value); };
    return cell;
}

template <typename T>
Detail::Field number(std::string label, T& value)
{
    return { std::move(label), { numberCell(value) } };
}

Detail::Field vector2(std::string label, Vector2& value)
{
    return { std::move(label), { numberCell(value.x, "X"), numberCell(value.y, "Y") } };
}

Detail::Field color(std::string label, RGBA& value)
{
    return { std::move(label), { numberCell(value.r, "R"), numberCell(value.g, "G"), numberCell(value.b, "B"), numberCell(value.a, "A") } };
}

Detail::Field text(std::string label, std::string& value, bool allowEmpty = true)
{
    Detail::Cell cell;
    cell.get = [&value]() { return value; };
    cell.set = [&value, allowEmpty](const std::string& input) {
        if (!allowEmpty && trim(input).empty())
            return false;
        value = input;
        return true;
    };
    return { std::move(label), { std::move(cell) } };
}

Detail::Field texture(std::string label, std::string& value)
{
    Detail::Field field = text(std::move(label), value);

    field.cells.front().acceptsImage = true;
    return field;
}

bool isImage(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();

    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".webp";
}

std::string texturePathFor(const std::filesystem::path& absolute)
{
    std::error_code error;
    const std::filesystem::path relative = std::filesystem::relative(absolute, std::filesystem::current_path(), error);

    if (!error && !relative.empty() && *relative.begin() != "..")
        return relative.generic_string();
    return absolute.string();
}

Detail::Field toggle(std::string label, bool& value)
{
    Detail::Cell cell;
    cell.get = [&value]() { return std::string(value ? "true" : "false"); };
    cell.click = [&value]() { value = !value; };
    return { std::move(label), { std::move(cell) } };
}

template <typename E>
Detail::Field choice(std::string label, E& value, std::vector<std::string> names)
{
    Detail::Cell cell;
    cell.get = [&value, names]() {
        const auto index = static_cast<std::size_t>(value);
        return index < names.size() ? names[index] : std::to_string(index);
    };
    cell.click = [&value, count = names.size()]() {
        value = static_cast<E>((static_cast<std::size_t>(value) + 1) % count);
    };
    return { std::move(label), { std::move(cell) } };
}

Detail::Field key(std::string label, Event::Key& value)
{
    Detail::Cell cell;
    cell.get = [&value]() {
        const auto index = static_cast<std::size_t>(value);
        return index < keyNames().size() ? keyNames()[index] : std::to_string(index);
    };
    cell.set = [&value](const std::string& input) {
        const std::string wanted = trim(input);
        const auto& names = keyNames();
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (sameText(names[i], wanted)) {
                value = static_cast<Event::Key>(i);
                return true;
            }
        }
        return false;
    };
    return { std::move(label), { std::move(cell) } };
}

Detail::Field info(std::string label, std::string value)
{
    Detail::Cell cell;
    cell.get = [value = std::move(value)]() { return value; };
    return { std::move(label), { std::move(cell) } };
}

// ---------------------------------------------------------------------------
// Animator: the animations list is edited directly in the panel
// ---------------------------------------------------------------------------

// Clickable button (no value to type)
Detail::Field button(std::string label, std::string text, std::function<void()> onClick)
{
    Detail::Cell cell;
    cell.get = [text = std::move(text)]() { return text; };
    cell.click = std::move(onClick);
    return { std::move(label), { std::move(cell) } };
}

// List of texts typed with commas: "a.png, b.png, c.png"
Detail::Field textList(std::string label, std::vector<std::string>& values, std::size_t& count)
{
    Detail::Cell cell;
    cell.get = [&values]() {
        std::string joined;
        for (const std::string& value : values)
            joined += (joined.empty() ? "" : ", ") + value;
        return joined;
    };
    cell.set = [&values, &count](const std::string& input) {
        std::vector<std::string> parsed;
        std::size_t start = 0;
        while (start <= input.size()) {
            const std::size_t comma = std::min(input.find(',', start), input.size());
            const std::string item = trim(input.substr(start, comma - start));
            if (!item.empty())
                parsed.push_back(item);
            start = comma + 1;
        }
        values = std::move(parsed);
        count = std::max<std::size_t>(1, values.size()); // one frame per image
        return true;
    };
    return { std::move(label), { std::move(cell) } };
}

std::vector<std::string> animationNames(const Animator& animator)
{
    std::vector<std::string> names;
    for (const auto& [name, animation] : animator.animations)
        names.push_back(name);
    std::sort(names.begin(), names.end()); // stable order on screen (unordered_map has none)
    return names;
}

// "Current" animation: a click switches to the next one
Detail::Field currentAnimation(Animator& animator)
{
    Detail::Cell cell;
    cell.get = [&animator]() {
        return animator.getCurrentAnimation() != nullptr ? animator.currentAnimation : std::string("(none)");
    };
    cell.click = [&animator]() {
        const std::vector<std::string> names = animationNames(animator);
        if (names.empty())
            return;
        auto it = std::upper_bound(names.begin(), names.end(), animator.currentAnimation);
        const bool wasPlaying = animator.playing;
        animator.play(it == names.end() ? names.front() : *it, true);
        animator.playing = wasPlaying;
    };
    return { "Current", { std::move(cell) } };
}

// "New animation": type a name and press Enter
Detail::Field newAnimation(Animator& animator)
{
    static const std::string PLACEHOLDER = "type a name + Enter";
    Detail::Cell cell;
    cell.get = []() { return PLACEHOLDER; };
    cell.set = [&animator](const std::string& input) {
        const std::string name = trim(input);
        if (name.empty() || name == PLACEHOLDER || animator.animations.count(name) != 0)
            return false;
        animator.addAnimation(name, Animation{});
        if (animator.getCurrentAnimation() == nullptr)
            animator.play(name, true); // the first animation becomes the current one
        return true;
    };
    return { "New anim", { std::move(cell) } };
}

void describeAnimation(std::vector<Detail::Field>& fields, Animator& animator, const std::string& name)
{
    Animation& a = animator.animations.at(name);

    fields.push_back(button("[" + name + "]", "Remove", [&animator, name]() {
        if (animator.currentAnimation == name)
            animator.stop();
        animator.animations.erase(name);
    }));
    fields.push_back(toggle(" Use sheet", a.isSpriteSheet));
    if (a.isSpriteSheet) {
        fields.push_back(texture(" Sheet", a.spriteSheetPath)); // accepts an image dropped from the assets
        fields.push_back(vector2(" Frame size", a.frameSize));
        fields.push_back(number(" Columns", a.columns));
        fields.push_back(number(" 1st frame", a.firstFrame));
        fields.push_back(number(" Frames", a.frameCount));
        fields.push_back(vector2(" Draw size", a.displaySize));
    } else {
        fields.push_back(textList(" Images", a.spritePaths, a.frameCount));
        fields.push_back(vector2(" Size", a.frameSize));
    }
    fields.push_back(number(" Frame time", a.frameDuration));
    fields.push_back(toggle(" Loop", a.loop));
}

std::vector<Detail::Field> describeAnimator(Animator& animator)
{
    std::vector<Detail::Field> fields = {
        currentAnimation(animator),
        toggle("Playing", animator.playing),
        info("Frame", std::to_string(animator.currentFrame)),
    };
    for (const std::string& name : animationNames(animator))
        describeAnimation(fields, animator, name);
    fields.push_back(newAnimation(animator));
    return fields;
}

const char *componentName(ComponentType type)
{
    switch (type) {
        case ComponentType::AddHealth: return "Add Health";
        case ComponentType::Collider: return "Collider";
        case ComponentType::Health: return "Health";
        case ComponentType::Orbit: return "Orbit";
        case ComponentType::Poison: return "Poison";
        case ComponentType::RegenHealth: return "Regen Health";
        case ComponentType::RemoveHealth: return "Remove Health";
        case ComponentType::Sinus: return "Sinus";
        case ComponentType::Spiral: return "Spiral";
        case ComponentType::Sprite: return "Sprite";
        case ComponentType::Transform: return "Transform";
        case ComponentType::Velocity: return "Velocity";
        case ComponentType::Wither: return "Wither";
        case ComponentType::Animator: return "Animator";
        case ComponentType::Owner: return "Owner";
        case ComponentType::Controllable: return "Controllable";
        case ComponentType::Name: return "Name";
        case ComponentType::Camera: return "Camera";
        case ComponentType::Music: return "Music";
        case ComponentType::Script: return "Script";
        case ComponentType::ENUM_END: break;
    }
    return "Unknown";
}

std::vector<Detail::Field> describe(IComponent& component)
{
    switch (component.getType()) {
        case ComponentType::Name: {
            auto& c = static_cast<Name&>(component);
            return { text("Name", c.name, false) };
        }
        case ComponentType::Transform: {
            auto& c = static_cast<Transform&>(component);
            return { vector2("Position", c.position), vector2("Scale", c.scale), number("Rotation", c.rotation) };
        }
        case ComponentType::Velocity: {
            auto& c = static_cast<Velocity&>(component);
            return { vector2("Velocity", c.velocity) };
        }
        case ComponentType::Sprite: {
            auto& c = static_cast<Sprite&>(component);
            return { texture("Texture", c.texturePath), vector2("Size", c.size) };
        }
        case ComponentType::Collider: {
            auto& c = static_cast<Collider&>(component);
            return { vector2("Size", c.size), choice("Shape", c.colliderShape, { "Box", "Circle" }) };
        }
        case ComponentType::Health: {
            auto& c = static_cast<Health&>(component);
            return { number("Current", c.currentHealth), number("Max", c.maxHealth) };
        }
        case ComponentType::AddHealth: {
            auto& c = static_cast<AddHealth&>(component);
            return { number("Amount", c.regenAmount) };
        }
        case ComponentType::RemoveHealth: {
            auto& c = static_cast<RemoveHealth&>(component);
            return { number("Hit Damage", c.hitDamage) };
        }
        case ComponentType::RegenHealth: {
            auto& c = static_cast<RegenHealth&>(component);
            return { number("Amount", c.regenAmount), number("Cooldown", c.cooldown), number("Duration", c.duration) };
        }
        case ComponentType::Poison: {
            auto& c = static_cast<Poison&>(component);
            return { number("Damage", c.damage), number("Cooldown", c.cooldown), number("Duration", c.duration) };
        }
        case ComponentType::Wither: {
            auto& c = static_cast<Wither&>(component);
            return { number("Damage", c.damage), number("Cooldown", c.cooldown), number("Duration", c.duration) };
        }
        case ComponentType::Orbit: {
            auto& c = static_cast<Orbit&>(component);
            return { vector2("Center", c.center), number("Target", c.targetEntity), number("Radius", c.radius),
                number("Angle", c.angle), number("Angle Speed", c.angleSpeed) };
        }
        case ComponentType::Sinus: {
            auto& c = static_cast<Sinus&>(component);
            return { vector2("Center", c.center), number("Target", c.targetEntity), number("Distance", c.distance),
                number("Current", c.curr), number("Speed", c.speed) };
        }
        case ComponentType::Spiral: {
            auto& c = static_cast<Spiral&>(component);
            return { vector2("Center", c.center), number("Target", c.targetEntity), number("Radius", c.radius),
                number("Expand Speed", c.expandSpeed), number("Angle", c.angle), number("Angle Speed", c.angleSpeed) };
        }
        case ComponentType::Animator:
            return describeAnimator(static_cast<Animator&>(component));
        case ComponentType::Owner: {
            auto& c = static_cast<Owner&>(component);
            return { choice("Team", c.team, { "Player", "Enemy" }) };
        }
        case ComponentType::Controllable: {
            auto& c = static_cast<Controllable&>(component);
            return { key("Up", c.up), key("Down", c.down), key("Left", c.left), key("Right", c.right),
                number("Speed", c.speed), vector2("Size", c.size), color("Color", c.color), texture("Image", c.image),
                number("Bound X", c.boundX), number("Bound Y", c.boundY) };
        }
        case ComponentType::Camera: {
            auto& c = static_cast<Camera&>(component);
            return { number("Bound X", c.boundX), number("Bound Y", c.boundY) };
        }
        case ComponentType::Music: {
            auto& c = static_cast<Music&>(component);
            return { text("File", c.path),
                choice("State", c.state, { "Playing", "Paused", "Stopped" }),
                number("Volume", c.volume),
                number("Pitch", c.pitch),
                toggle("Loop", c.loop) };
        }
        case ComponentType::Script: {
            auto& c = static_cast<Script&>(component);
            return { text("Path", c.path) };
        }
        case ComponentType::ENUM_END:
            break;
    }
    return {};
}

int displayRank(ComponentType type)
{
    if (type == ComponentType::Name)
        return -2;
    if (type == ComponentType::Transform)
        return -1;
    return static_cast<int>(type);
}

} // namespace
} // namespace OrByte

OrByte::Detail::Detail(World& world, const ComponentFactories& factories, int width)
    : Widget("Details", width), _world(world), _factories(factories)
{
}

std::vector<OrByte::ComponentType> OrByte::Detail::_addableComponents(int entityId) const
{
    std::vector<ComponentType> types;

    for (std::size_t i = 0; i < static_cast<std::size_t>(ComponentType::ENUM_END); ++i) {
        const auto type = static_cast<ComponentType>(i);
        const auto factory = this->_factories.find(type);

        if (factory == this->_factories.end() || factory->second == nullptr)
            continue;
        if (this->_world.getComponent(entityId, type) != nullptr)
            continue;
        types.push_back(type);
    }
    std::sort(types.begin(), types.end(), [](ComponentType a, ComponentType b) {
        return std::string(componentName(a)) < std::string(componentName(b));
    });
    return types;
}

void OrByte::Detail::_addComponent(int entityId, ComponentType type)
{
    const auto factory = this->_factories.find(type);
    if (factory == this->_factories.end() || factory->second == nullptr)
        return;

    std::unique_ptr<IComponent> component(factory->second());
    if (component == nullptr || component->getType() != type)
        return;
    this->_world.addComponentToEntity(entityId, std::move(component));
    this->_collapsed.erase(type);
}

void OrByte::Detail::_removeComponent(int entityId, ComponentType type)
{
    if (!World::isComponentRemovable(type))
        return;
    if (this->_edit.active && this->_edit.type == type)
        this->_cancelEdit();
    this->_world.removeComponentFromEntity(entityId, type);
    this->_clampScroll();
}

std::vector<OrByte::Detail::Row> OrByte::Detail::_buildRows(int entityId)
{
    std::vector<Row> rows;
    const float top = static_cast<float>(HEADER + 4);
    float y = top - this->_scroll;
    std::vector<ComponentType> types = this->_world.getComponentTypesOfEntity(entityId);

    std::sort(types.begin(), types.end(), [](ComponentType a, ComponentType b) { return displayRank(a) < displayRank(b); });
    for (ComponentType type : types) {
        IComponent *component = this->_world.getComponent(entityId, type);
        if (component == nullptr)
            continue;

        rows.push_back(Row{ RowKind::Section, y, static_cast<float>(SECTION_HEIGHT), type, -1, Field{ componentName(type), {} } });
        y += static_cast<float>(SECTION_HEIGHT);
        if (this->_collapsed.count(type) == 0) {
            std::vector<Field> fields = describe(*component);
            for (std::size_t i = 0; i < fields.size(); ++i) {
                rows.push_back(Row{ RowKind::Value, y, static_cast<float>(ROW_HEIGHT), type, static_cast<int>(i), std::move(fields[i]) });
                y += static_cast<float>(ROW_HEIGHT);
            }
        }
        y += 4.f;
    }

    y += 6.f;
    rows.push_back(Row{ RowKind::AddButton, y, static_cast<float>(SECTION_HEIGHT), ComponentType::ENUM_END, -1, Field{ "+ Add Component", {} } });
    y += static_cast<float>(SECTION_HEIGHT);
    if (this->_addMenuOpen) {
        for (ComponentType type : this->_addableComponents(entityId)) {
            rows.push_back(Row{ RowKind::AddChoice, y, static_cast<float>(ROW_HEIGHT), type, -1, Field{ componentName(type), {} } });
            y += static_cast<float>(ROW_HEIGHT);
        }
    }
    this->_contentHeight = y + this->_scroll - top + static_cast<float>(PADDING);
    return rows;
}

Rect OrByte::Detail::_cellRect(const Row& row, std::size_t cellIndex, int width) const
{
    const int labelWidth = std::clamp(static_cast<int>(static_cast<float>(width) * 0.35f), 70, 150);
    const int count = static_cast<int>(std::max<std::size_t>(1, row.field.cells.size()));
    const int available = std::max(count, width - labelWidth - PADDING - CELL_GAP * (count - 1));
    const int cellWidth = available / count;

    return Rect(labelWidth + static_cast<int>(cellIndex) * (cellWidth + CELL_GAP),
        static_cast<int>(row.y) + 2, cellWidth, static_cast<int>(row.height) - 4);
}

Rect OrByte::Detail::_removeRect(const Row& row, int width) const
{
    const int size = static_cast<int>(row.height) - 8;
    return Rect(width - PADDING - size, static_cast<int>(row.y) + 4, size, size);
}

bool OrByte::Detail::_isEditing(const Row& row, std::size_t cellIndex) const
{
    return this->_edit.active && row.kind == RowKind::Value && this->_edit.type == row.type && this->_edit.fieldIndex == row.fieldIndex
        && this->_edit.cellIndex == static_cast<int>(cellIndex);
}

bool OrByte::Detail::_findImageCell(Vector2 localMouse, int& entityId, Row& found, std::size_t& cellIndex)
{
    this->_syncSelection();
    entityId = this->_world.getSelectedEntity().value_or(-1);
    if (entityId < 0 || !this->_world.entityExists(entityId) || localMouse.y < static_cast<float>(HEADER))
        return false;

    const int width = this->getContentRect().w;
    const int x = static_cast<int>(localMouse.x);
    const int y = static_cast<int>(localMouse.y);

    for (const Row& row : this->_buildRows(entityId)) {
        if (row.kind != RowKind::Value || localMouse.y < row.y || localMouse.y >= row.y + row.height)
            continue;
        for (std::size_t i = 0; i < row.field.cells.size(); ++i) {
            if (row.field.cells[i].acceptsImage && this->_cellRect(row, i, width).contains(x, y)) {
                found = row;
                cellIndex = i;
                return true;
            }
        }
        return false;
    }
    return false;
}

void OrByte::Detail::_syncSelection()
{
    const int selected = this->_world.getSelectedEntity().value_or(-1);

    if (this->_edit.active && this->_edit.entity != selected)
        this->_commitEdit();
    if (selected != this->_shownEntity) {
        this->_shownEntity = selected;
        this->_scroll = 0.f;
        this->_addMenuOpen = false;
    }
}

void OrByte::Detail::_clampScroll()
{
    const float visible = static_cast<float>(this->getContentRect().h - HEADER);
    const float maxScroll = std::max(0.f, this->_contentHeight - visible);

    this->_scroll = std::clamp(this->_scroll, 0.f, maxScroll);
}

void OrByte::Detail::_startEdit(int entityId, const Row& row, std::size_t cellIndex)
{
    this->_edit = Edit{ true, entityId, row.type, row.fieldIndex, static_cast<int>(cellIndex),
        row.field.cells[cellIndex].get(), true };
}

void OrByte::Detail::_commitEdit()
{
    if (!this->_edit.active)
        return;

    const Edit edit = std::move(this->_edit);
    this->_edit = Edit{};

    IComponent *component = this->_world.getComponent(edit.entity, edit.type);
    if (component == nullptr)
        return;

    std::vector<Field> fields = describe(*component);
    if (edit.fieldIndex < 0 || edit.fieldIndex >= static_cast<int>(fields.size()))
        return;

    std::vector<Cell>& cells = fields[static_cast<std::size_t>(edit.fieldIndex)].cells;
    if (edit.cellIndex < 0 || edit.cellIndex >= static_cast<int>(cells.size()))
        return;

    Cell& cell = cells[static_cast<std::size_t>(edit.cellIndex)];
    if (cell.set)
        cell.set(edit.text);
}

void OrByte::Detail::_cancelEdit()
{
    this->_edit = Edit{};
}

void OrByte::Detail::_editNextCell()
{
    const Edit previous = this->_edit;
    bool passed = false;

    this->_commitEdit();
    for (const Row& row : this->_buildRows(previous.entity)) {
        if (row.kind != RowKind::Value)
            continue;
        for (std::size_t i = 0; i < row.field.cells.size(); ++i) {
            if (!passed) {
                passed = row.type == previous.type && row.fieldIndex == previous.fieldIndex
                    && static_cast<int>(i) == previous.cellIndex;
                continue;
            }
            const Cell& cell = row.field.cells[i];
            if (cell.set && !cell.click) {
                this->_startEdit(previous.entity, row, i);
                return;
            }
        }
    }
}

bool OrByte::Detail::onEvent(const Event& event, Vector2 localMouse)
{
    this->_syncSelection();

    switch (event.type) {
        case Event::Type::MouseWheelScrolled:
            this->_scroll -= static_cast<float>(event.wheelDelta) * static_cast<float>(ROW_HEIGHT * 2);
            this->_clampScroll();
            return true;
        case Event::Type::MouseButtonPressed:
            if (event.mouseButton != Event::MouseButton::Left)
                return false;
            return this->_onClick(localMouse);
        case Event::Type::TextEntered:
            return this->_onText(event);
        case Event::Type::KeyPressed:
            return this->_onKey(event);
        case Event::Type::KeyReleased:
            return this->_edit.active;
        default:
            return false;
    }
}

bool OrByte::Detail::_onClick(Vector2 localMouse)
{
    if (localMouse.y < static_cast<float>(HEADER)) {
        this->_commitEdit();
        return false;
    }

    const int entityId = this->_world.getSelectedEntity().value_or(-1);
    if (entityId < 0 || !this->_world.entityExists(entityId)) {
        this->_commitEdit();
        return true;
    }

    const int width = this->getContentRect().w;
    for (const Row& row : this->_buildRows(entityId)) {
        if (localMouse.y < row.y || localMouse.y >= row.y + row.height)
            continue;

        if (row.kind == RowKind::Value) {
            for (std::size_t i = 0; i < row.field.cells.size(); ++i)
                if (this->_isEditing(row, i)
                    && this->_cellRect(row, i, width).contains(static_cast<int>(localMouse.x), static_cast<int>(localMouse.y)))
                    return true;
        }
        this->_commitEdit();
        if (row.kind == RowKind::AddButton) {
            this->_addMenuOpen = !this->_addMenuOpen && !this->_addableComponents(entityId).empty();
            this->_clampScroll();
            return true;
        }
        if (row.kind == RowKind::AddChoice) {
            this->_addComponent(entityId, row.type);
            this->_addMenuOpen = false;
            this->_clampScroll();
            return true;
        }
        this->_addMenuOpen = false;
        if (row.kind == RowKind::Section) {
            const Rect remove = this->_removeRect(row, width);
            if (World::isComponentRemovable(row.type)
                && remove.contains(static_cast<int>(localMouse.x), static_cast<int>(localMouse.y))) {
                this->_removeComponent(entityId, row.type);
                return true;
            }
            if (!this->_collapsed.erase(row.type))
                this->_collapsed.insert(row.type);
            this->_clampScroll();
            return true;
        }
        for (std::size_t i = 0; i < row.field.cells.size(); ++i) {
            const Rect rect = this->_cellRect(row, i, width);
            if (!rect.contains(static_cast<int>(localMouse.x), static_cast<int>(localMouse.y)))
                continue;
            const Cell& cell = row.field.cells[i];
            if (cell.click)
                cell.click();
            else if (cell.set)
                this->_startEdit(entityId, row, i);
            return true;
        }
        return true;
    }
    this->_commitEdit();
    this->_addMenuOpen = false;
    return true;
}

bool OrByte::Detail::_onText(const Event& event)
{
    if (!this->_edit.active)
        return false;

    const unsigned character = event.keyChar;
    if (character == '\t') {
        this->_editNextCell();
    } else if (character >= 32 && character <= 126) {
        if (this->_edit.selectAll)
            this->_edit.text.clear();
        this->_edit.selectAll = false;
        this->_edit.text.push_back(static_cast<char>(character));
    }
    return true;
}

bool OrByte::Detail::_onKey(const Event& event)
{
    if (!this->_edit.active) {
        if (this->_addMenuOpen && event.key == Event::Key::Escape) {
            this->_addMenuOpen = false;
            return true;
        }
        return false;
    }

    switch (event.key) {
        case Event::Key::Backspace:
            if (this->_edit.selectAll)
                this->_edit.text.clear();
            else if (!this->_edit.text.empty())
                this->_edit.text.pop_back();
            this->_edit.selectAll = false;
            break;
        case Event::Key::Left:
        case Event::Key::Right:
            this->_edit.selectAll = false;
            break;
        case Event::Key::Enter:
            this->_commitEdit();
            break;
        case Event::Key::Escape:
            this->_cancelEdit();
            break;
        default:
            break;
    }
    return true;
}

void OrByte::Detail::_drawRow(const IGraphic& graphic, const Row& row, int width) const
{
    const float w = static_cast<float>(width);

    if (row.kind == RowKind::Section) {
        const std::string arrow = (this->_collapsed.count(row.type) != 0) ? "> " : "v ";
        fillRect(graphic, 0.f, row.y, w, row.height - 1.f, SECTION_COLOR);
        drawLabel(graphic, static_cast<float>(PADDING), row.y + 4.f, arrow + row.field.label, SECTION_FONT, TEXT_COLOR);
        if (World::isComponentRemovable(row.type)) {
            const Rect remove = this->_removeRect(row, width);
            fillRect(graphic, static_cast<float>(remove.x), static_cast<float>(remove.y), static_cast<float>(remove.w), static_cast<float>(remove.h),
                REMOVE_COLOR, 1, BORDER_COLOR);
            drawLabel(graphic, static_cast<float>(remove.x) + static_cast<float>(remove.w) / 2.f - 3.5f, static_cast<float>(remove.y) - 2.f,
                "x", SECTION_FONT, TEXT_COLOR);
        }
        return;
    }
    if (row.kind == RowKind::AddButton) {
        const bool available = !this->_addableComponents(this->_shownEntity).empty();
        fillRect(graphic, static_cast<float>(PADDING), row.y, w - static_cast<float>(PADDING * 2), row.height - 2.f,
            available ? BUTTON_COLOR : READONLY_COLOR, 1, this->_addMenuOpen ? EDIT_BORDER_COLOR : BORDER_COLOR);
        drawLabel(graphic, static_cast<float>(PADDING * 2), row.y + 4.f, available ? row.field.label : "No component to add",
            SECTION_FONT, available ? TEXT_COLOR : MUTED_COLOR);
        return;
    }
    if (row.kind == RowKind::AddChoice) {
        fillRect(graphic, static_cast<float>(PADDING), row.y, w - static_cast<float>(PADDING * 2), row.height,
            CELL_COLOR, 1, BORDER_COLOR);
        drawLabel(graphic, static_cast<float>(PADDING * 3), row.y + (row.height - static_cast<float>(FIELD_FONT)) / 2.f - 2.f,
            row.field.label, FIELD_FONT, TEXT_COLOR);
        return;
    }

    const Rect first = this->_cellRect(row, 0, width);
    const float labelX = static_cast<float>(PADDING + 8);
    const float textY = row.y + (row.height - static_cast<float>(FIELD_FONT)) / 2.f - 2.f;
    drawLabel(graphic, labelX, textY, fit(row.field.label, static_cast<float>(first.x) - labelX - 4.f, FIELD_FONT), FIELD_FONT, MUTED_COLOR);

    const bool blink = (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count() / 500) % 2 == 0;

    for (std::size_t i = 0; i < row.field.cells.size(); ++i) {
        const Cell& cell = row.field.cells[i];
        const Rect rect = this->_cellRect(row, i, width);
        const bool editing = this->_isEditing(row, i);
        const bool readOnly = !cell.set && !cell.click;
        float textX = static_cast<float>(rect.x) + 4.f;

        fillRect(graphic, static_cast<float>(rect.x), static_cast<float>(rect.y), static_cast<float>(rect.w), static_cast<float>(rect.h),
            readOnly ? READONLY_COLOR : CELL_COLOR, 1, editing ? EDIT_BORDER_COLOR : BORDER_COLOR);
        if (!cell.tag.empty()) {
            drawLabel(graphic, textX, textY, cell.tag, FIELD_FONT, tagColor(cell.tag));
            textX += 11.f;
        }

        const float room = static_cast<float>(rect.x + rect.w) - textX - 2.f;
        const bool selected = editing && this->_edit.selectAll && !this->_edit.text.empty();
        const std::string value = editing
            ? fit(this->_edit.text + (blink && !selected ? "|" : ""), room, FIELD_FONT, true)
            : fit(cell.get(), room, FIELD_FONT);
        if (selected) {
            const float selectionWidth = std::min(room, static_cast<float>(value.size()) * static_cast<float>(FIELD_FONT) * GLYPH_RATIO);
            fillRect(graphic, textX - 1.f, static_cast<float>(rect.y) + 2.f, selectionWidth + 2.f, static_cast<float>(rect.h) - 4.f, SELECTION_COLOR);
        }
        drawLabel(graphic, textX, textY, value, FIELD_FONT, readOnly ? MUTED_COLOR : TEXT_COLOR);
    }
}

void OrByte::Detail::draw(const IGraphic& graphic, Vector2u size)
{
    const int width = static_cast<int>(size.x);
    const float w = static_cast<float>(size.x);
    const float h = static_cast<float>(size.y);
    const int entityId = this->_world.getSelectedEntity().value_or(-1);
    const bool hasEntity = entityId >= 0 && this->_world.entityExists(entityId);

    this->_syncSelection();
    fillRect(graphic, 0.f, 0.f, w, h, BG_COLOR, 1, BORDER_COLOR);

    if (hasEntity) {
        this->_clampScroll();
        for (const Row& row : this->_buildRows(entityId)) {
            if (row.y + row.height < static_cast<float>(HEADER) || row.y > h)
                continue;
            this->_drawRow(graphic, row, width);
        }
    } else {
        drawLabel(graphic, static_cast<float>(PADDING), static_cast<float>(HEADER + 8), "Select an entity in the Hierarchy", FIELD_FONT, MUTED_COLOR);
    }

    fillRect(graphic, 0.f, 0.f, w, static_cast<float>(HEADER), HEADER_COLOR);
    drawLabel(graphic, 6.f, 3.f, this->getTitle(), 14, TEXT_COLOR);
    if (hasEntity) {
        const std::string id = "id: " + std::to_string(entityId);
        drawLabel(graphic, w - static_cast<float>(id.size() * 7 + PADDING), 5.f, id, 12, MUTED_COLOR);
    }
}

bool OrByte::Detail::canAccept(const DragPayload& payload, Vector2 localMouse)
{
    int entityId = -1;
    Row row;
    std::size_t cellIndex = 0;

    if (payload.kind != "asset" || !isImage(payload.path))
        return false;
    return this->_findImageCell(localMouse, entityId, row, cellIndex);
}

void OrByte::Detail::onDrop(const DragPayload& payload, Vector2 localMouse)
{
    int entityId = -1;
    Row row;
    std::size_t cellIndex = 0;

    if (payload.kind != "asset" || !isImage(payload.path))
        return;
    if (!this->_findImageCell(localMouse, entityId, row, cellIndex))
        return;

    this->_commitEdit();
    if (row.field.cells[cellIndex].set)
        row.field.cells[cellIndex].set(texturePathFor(payload.path));
}
