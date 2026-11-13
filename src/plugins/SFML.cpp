/**
 * @file SFML.cpp
 * @brief SFML
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#include "constants.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/SFML.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <SFML/Graphics.hpp>
#include <thread>
#include <utility>

namespace {
    Vector2 getWindowRatio(const sf::Vector2u windowSize)
    {
        const float ratio = std::min(static_cast<float>(windowSize.x) / static_cast<float>(OrByte::constants::SERVER_WINDOW_X),
                static_cast<float>(windowSize.y) / static_cast<float>(OrByte::constants::SERVER_WINDOW_Y));

        return { ratio, ratio };
    }

    Vector2 getWindowOffset(const sf::Vector2u windowSize)
    {
        const Vector2 windowRatio = getWindowRatio(windowSize);

        return { (static_cast<float>(windowSize.x) - static_cast<float>(OrByte::constants::SERVER_WINDOW_X) * windowRatio.x) / 2,
                (static_cast<float>(windowSize.y) - static_cast<float>(OrByte::constants::SERVER_WINDOW_Y) * windowRatio.y) / 2 };
    }

    Vector2 getPositionFromWindow(const sf::Vector2i position, const sf::Vector2u windowSize)
    {
        const Vector2 windowRatio = getWindowRatio(windowSize);
        const Vector2 offset = getWindowOffset(windowSize);

        return { (static_cast<float>(position.x) - offset.x) / windowRatio.x, (static_cast<float>(position.y) - offset.y) / windowRatio.y };
    }

    sf::Color colorNormalize(const OrByte::RGBA color)
    {
        return sf::Color(
            color.r,
            color.g,
            color.b,
            color.a
        );
    }

}

void OrByte::SFML::open(const std::size_t width, const std::size_t height, std::string title)
{
    if (this->_isOpen)
        return;
    this->_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(sf::Vector2u(width, height), 32), title, sf::Style::Default);
    this->_window->setFramerateLimit(constants::FPS_LIMIT);

    this->_width = width;
    this->_height = height;
    this->_nextFrameTime = std::chrono::steady_clock::now();

    this->_isOpen = true;
}

void OrByte::SFML::close()
{
    if (!this->_isOpen)
        return;
    this->_window->close();
    this->_window.reset();
    this->_isOpen = false;
}

std::queue<OrByte::Event> OrByte::SFML::listen() const
{
    std::queue<Event> events;

    if (!this->_isOpen || !this->_window)
        return events;

    const auto mapKey = [](sf::Keyboard::Key key, Event& event) {
        switch (key) {
            case sf::Keyboard::Key::Left:
                event.key = Event::Key::Left; break;
            case sf::Keyboard::Key::Right:
                event.key = Event::Key::Right; break;
            case sf::Keyboard::Key::Down:
                event.key = Event::Key::Down; break;
            case sf::Keyboard::Key::Up:
                event.key = Event::Key::Up; break;
            case sf::Keyboard::Key::Enter:
                event.key = Event::Key::Enter; break;
            case sf::Keyboard::Key::Escape:
                event.key = Event::Key::Escape; break;
            case sf::Keyboard::Key::Space:
                event.key = Event::Key::Space; break;
            case sf::Keyboard::Key::Backspace:
                event.key = Event::Key::Backspace; break;
            case sf::Keyboard::Key::Delete:
                event.key = Event::Key::Delete; break;
            case sf::Keyboard::Key::Insert:
                event.key = Event::Key::Insert; break;
            case sf::Keyboard::Key::A:
                event.key = Event::Key::A;
                break;
            case sf::Keyboard::Key::B:
                event.key = Event::Key::B;
                break;
            case sf::Keyboard::Key::C:
                event.key = Event::Key::C;
                break;
            case sf::Keyboard::Key::D:
                event.key = Event::Key::D;
                break;
            case sf::Keyboard::Key::E:
                event.key = Event::Key::E;
                break;
            case sf::Keyboard::Key::F:
                event.key = Event::Key::F;
                break;
            case sf::Keyboard::Key::G:
                event.key = Event::Key::G;
                break;
            case sf::Keyboard::Key::H:
                event.key = Event::Key::H;
                break;
            case sf::Keyboard::Key::I:
                event.key = Event::Key::I;
                break;
            case sf::Keyboard::Key::J:
                event.key = Event::Key::J;
                break;
            case sf::Keyboard::Key::K:
                event.key = Event::Key::K;
                break;
            case sf::Keyboard::Key::L:
                event.key = Event::Key::L;
                break;
            case sf::Keyboard::Key::M:
                event.key = Event::Key::M;
                break;
            case sf::Keyboard::Key::N:
                event.key = Event::Key::N;
                break;
            case sf::Keyboard::Key::O:
                event.key = Event::Key::O;
                break;
            case sf::Keyboard::Key::P:
                event.key = Event::Key::P;
                break;
            case sf::Keyboard::Key::Q:
                event.key = Event::Key::Q;
                break;
            case sf::Keyboard::Key::R:
                event.key = Event::Key::R;
                break;
            case sf::Keyboard::Key::S:
                event.key = Event::Key::S;
                break;
            case sf::Keyboard::Key::T:
                event.key = Event::Key::T;
                break;
            case sf::Keyboard::Key::U:
                event.key = Event::Key::U;
                break;
            case sf::Keyboard::Key::V:
                event.key = Event::Key::V;
                break;
            case sf::Keyboard::Key::W:
                event.key = Event::Key::W;
                break;
            case sf::Keyboard::Key::X:
                event.key = Event::Key::X;
                break;
            case sf::Keyboard::Key::Y:
                event.key = Event::Key::Y;
                break;
            case sf::Keyboard::Key::Z:
                event.key = Event::Key::Z;
                break;
            default: break;
        }
    };

    const auto mapMouseButton = [](sf::Mouse::Button button, Event& event) {
        switch (button) {
            case sf::Mouse::Button::Left:
                event.mouseButton = Event::MouseButton::Left;
                break;
            case sf::Mouse::Button::Right:
                event.mouseButton = Event::MouseButton::Right;
                break;
            case sf::Mouse::Button::Middle:
                event.mouseButton = Event::MouseButton::Middle;
                break;
            default:
                break;
        }
    };

    const auto setMousePosition = [this](int x, int y, Event& event) {
        event.windowX = x;
        event.windowY = y;

        Vector2 mousePos = getPositionFromWindow(
            {event.windowX, event.windowY},
            this->_window->getSize()
        );

        event.mouseX = static_cast<int>(mousePos.x);
        event.mouseY = static_cast<int>(mousePos.y);
    };

    while (const auto sfmlEvent = this->_window->pollEvent()) {
        Event event;

        if (sfmlEvent->is<sf::Event::Closed>()) {
            event.type = Event::Type::Closed;
            events.push(event);
        }
        else if (const auto* resized = sfmlEvent->getIf<sf::Event::Resized>()) {
            event.type = Event::Type::Resized;
            event.width = resized->size.x;
            event.height = resized->size.y;

            this->_window->setView(sf::View(sf::FloatRect({0.f, 0.f},
                { static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) })));

            events.push(event);
        }
        else if (const auto* keyPressed = sfmlEvent->getIf<sf::Event::KeyPressed>()) {
            event.type = Event::Type::KeyPressed;
            mapKey(keyPressed->code, event);
            events.push(event);
        }
        else if (const auto* keyReleased = sfmlEvent->getIf<sf::Event::KeyReleased>()) {
            event.type = Event::Type::KeyReleased;
            mapKey(keyReleased->code, event);
            events.push(event);
        }
        else if (const auto* textEntered = sfmlEvent->getIf<sf::Event::TextEntered>()) {
            event.type = Event::Type::TextEntered;
            event.keyChar = static_cast<char>(textEntered->unicode);
            events.push(event);
        }
        else if (const auto* mousePressed = sfmlEvent->getIf<sf::Event::MouseButtonPressed>()) {
            event.type = Event::Type::MouseButtonPressed;
            setMousePosition(mousePressed->position.x, mousePressed->position.y, event);
            mapMouseButton(mousePressed->button, event);
            events.push(event);
        }
        else if (const auto* mouseReleased = sfmlEvent->getIf<sf::Event::MouseButtonReleased>()) {
            event.type = Event::Type::MouseButtonReleased;
            setMousePosition(mouseReleased->position.x, mouseReleased->position.y, event);
            mapMouseButton(mouseReleased->button, event);
            events.push(event);
        }
        else if (const auto* mouseMoved = sfmlEvent->getIf<sf::Event::MouseMoved>()) {
            event.type = Event::Type::MouseMoved;
            setMousePosition(mouseMoved->position.x, mouseMoved->position.y, event);
            events.push(event);
        }
        else if (const auto* mouseWheel = sfmlEvent->getIf<sf::Event::MouseWheelScrolled>()) {
            event.type = Event::Type::MouseWheelScrolled;
            setMousePosition(mouseWheel->position.x, mouseWheel->position.y, event);
            event.wheelDelta = mouseWheel->delta;
            events.push(event);
        }
    }

    return events;
}

void OrByte::SFML::clear() const
{
    if (!this->_isOpen || !this->_window)
        return;
    this->_window->clear(sf::Color::Black);
}

void OrByte::SFML::update()
{
    if (!this->_isOpen || !this->_window)
        return;
    std::this_thread::sleep_until(this->_nextFrameTime);
    sf::Vector2u windowSize = this->_window->getSize();
    this->_width = windowSize.x;
    this->_height = windowSize.y;
    this->_window->display();
    this->_nextFrameTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000 / constants::FPS_LIMIT);
}

void OrByte::SFML::drawRectangle(const Transform& transform, Vector2 size, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor) const
{
    sf::RectangleShape rectangle;

    if (!this->_isOpen || !this->_window)
        return;
    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();
    const sf::Vector2f sizeRatio(size.x * ratio.x, size.y * ratio.y);

    rectangle.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x, offset.y + transform.position.y * ratio.y));
    rectangle.setSize(sizeRatio);
    rectangle.setOrigin({ sizeRatio.x / 2, sizeRatio.y / 2 });
    rectangle.setRotation(sf::degrees(transform.rotation));
    rectangle.setFillColor(colorNormalize(fillColor));
    rectangle.setOutlineThickness(outlineThickness);
    rectangle.setOutlineColor(colorNormalize(outlineColor));

    this->_window->draw(rectangle);
}

void OrByte::SFML::drawCircle(const Transform& transform, float radius, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor) const
{
    sf::CircleShape circle;

    if (!this->_isOpen || !this->_window)
        return;
    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();
    const float radiusRatio = radius * ratio.y;

    circle.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x, offset.y + transform.position.y * ratio.y));
    circle.setRadius(radiusRatio);
    circle.setOrigin({radiusRatio, radiusRatio});
    circle.setRotation(sf::degrees(transform.rotation));
    circle.setFillColor(colorNormalize(fillColor));
    circle.setOutlineThickness(outlineThickness);
    circle.setOutlineColor(colorNormalize(outlineColor));

    this->_window->draw(circle);
}

const sf::Font *OrByte::SFML::getFont(const std::filesystem::path& path) const
{
    auto font = this->_fonts.find(path.string());

    if (font == this->_fonts.end()) {
        sf::Font loaded;
        if (!loaded.openFromFile(path))
            return nullptr;
        font = this->_fonts.emplace(path.string(), std::move(loaded)).first;
    }
    return &font->second;
}

const sf::Texture *OrByte::SFML::getTexture(const std::filesystem::path& path) const
{
    auto texture = this->_textures.find(path.string());

    if (texture == this->_textures.end()) {
        sf::Texture loaded;
        if (!loaded.loadFromFile(path.string()))
            return nullptr;
        texture = this->_textures.emplace(path.string(), std::move(loaded)).first;
    }
    return &texture->second;
}

void OrByte::SFML::drawImage(const Transform& transform, Vector2 size, std::filesystem::path path, bool contain, float opacity) const
{
    if (!this->_isOpen || !this->_window)
        return;
    const sf::Texture *texture = this->getTexture(path);
    if (texture == nullptr)
        return;

    const sf::Vector2u textureSize = texture->getSize();
    if (textureSize.x == 0 || textureSize.y == 0)
        return;

    sf::Sprite sprite(*texture);
    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();
    const float scaleX = (size.x * ratio.x) / static_cast<float>(textureSize.x);
    const float scaleY = (size.y * ratio.y) / static_cast<float>(textureSize.y);
    const float scale = contain ? std::min(scaleX, scaleY) : scaleY;
    const float clamped = std::clamp(opacity, 0.f, 1.f);

    sprite.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x,
                                    offset.y + transform.position.y * ratio.y));
    sprite.setScale({ scale, scale });
    sprite.setOrigin({ textureSize.x / 2.f, textureSize.y / 2.f });
    sprite.setRotation(sf::degrees(transform.rotation));
    sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(clamped * 255.f)));

    this->_window->draw(sprite);
}

void OrByte::SFML::drawImage(const Transform& transform, Vector2 size, std::filesystem::path path, Rect rect, bool contain, float opacity) const
{
    if (!this->_isOpen || !this->_window)
        return;
    const sf::Texture *texture = this->getTexture(path);
    if (texture == nullptr)
        return;

    if (rect.w <= 0 || rect.h <= 0)
        return;

    sf::Sprite sprite(*texture);
    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();
    const float scaleX = (size.x * ratio.x) / static_cast<float>(rect.w);
    const float scaleY = (size.y * ratio.y) / static_cast<float>(rect.h);
    const float scale = contain ? std::min(scaleX, scaleY) : scaleY;
    const float clamped = std::clamp(opacity, 0.f, 1.f);

    sprite.setTextureRect(sf::IntRect({ rect.x, rect.y }, { rect.w, rect.h }));
    sprite.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x,
                                    offset.y + transform.position.y * ratio.y));
    sprite.setScale({ scale, scale });
    sprite.setOrigin({ rect.w / 2.f, rect.h / 2.f });
    sprite.setRotation(sf::degrees(transform.rotation));
    sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(clamped * 255.f)));

    this->_window->draw(sprite);
}

Vector2 OrByte::SFML::drawText(const Transform& transform, std::string text, unsigned fontSize, float maxWidth, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor, std::filesystem::path font) const
{
    if (!this->_isOpen || !this->_window)
        return {0.f, 0.f};
    const sf::Font *sfmlFont = this->getFont(font);
    if (sfmlFont == nullptr)
        return {0.f, 0.f};

    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();

    sf::Text sfmlText(*sfmlFont, text, static_cast<unsigned>(static_cast<float>(fontSize) * ratio.y));

    sfmlText.setFillColor(colorNormalize(fillColor));
    sfmlText.setOutlineThickness(outlineThickness);
    sfmlText.setOutlineColor(colorNormalize(outlineColor));

    if (maxWidth > 0.f) {
        const float limit = maxWidth * ratio.x;

        if (sfmlText.getLocalBounds().size.x > limit) {
            const std::string ellipsis = "...";
            while (!text.empty()) {
                text.pop_back();
                sfmlText.setString(text + ellipsis);
                if (sfmlText.getLocalBounds().size.x <= limit)
                    break;
            }
            if (text.empty())
                sfmlText.setString(ellipsis);
        }
    }

    sfmlText.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x, offset.y + transform.position.y * ratio.y));
    this->_window->draw(sfmlText);

    const auto localBounds = sfmlText.getLocalBounds();
    return {localBounds.size.x, localBounds.size.y};
}

Vector2 OrByte::SFML::getTextSize(std::string text, unsigned fontSize, float maxWidth, std::filesystem::path font) const
{
    if (!this->_isOpen || !this->_window)
        return {0.f, 0.f};
    const sf::Font *sfmlFont = this->getFont(font);
    if (sfmlFont == nullptr)
        return {0.f, 0.f};

    const Vector2 ratio = this->currentRatio();

    sf::Text sfmlText(*sfmlFont, text, static_cast<unsigned>(static_cast<float>(fontSize) * ratio.y));

    if (maxWidth > 0.f) {
        const float limit = maxWidth * ratio.x;

        if (sfmlText.getLocalBounds().size.x > limit) {
            const std::string ellipsis = "...";
            while (!text.empty()) {
                text.pop_back();
                sfmlText.setString(text + ellipsis);
                if (sfmlText.getLocalBounds().size.x <= limit)
                    break;
            }
            if (text.empty())
                sfmlText.setString(ellipsis);
        }
    }

    const auto localBounds = sfmlText.getLocalBounds();
    return {localBounds.size.x, localBounds.size.y};
}

Vector2u OrByte::SFML::regionSize() const
{
    if (this->_regions.empty()) {
        sf::Vector2u windowSize = this->_window->getSize();
        return Vector2u(windowSize.x, windowSize.y);
    }
    return { static_cast<unsigned>(this->_regions.back().w),
        static_cast<unsigned>(this->_regions.back().h) };
}

void OrByte::SFML::applyRegionView() const
{
    const sf::Vector2f windowSize(this->_window->getSize());

    if (this->_regions.empty()) {
        this->_window->setView(sf::View(sf::FloatRect({0.f, 0.f}, {windowSize.x, windowSize.y})));
        return;
    }

    const Rect& rect = this->_regions.back();
    sf::View view(sf::FloatRect(
        {0.f, 0.f},
        {static_cast<float>(rect.w), static_cast<float>(rect.h)}
    ));

    view.setViewport(sf::FloatRect({rect.x / windowSize.x, rect.y / windowSize.y}, {rect.w / windowSize.x, rect.h / windowSize.y}));
    this->_window->setView(view);
}

std::pair<unsigned, unsigned> OrByte::SFML::getWindowSize() const noexcept
{
    sf::Vector2u windowSize = this->_window->getSize();

    return { windowSize.x, windowSize.y };
}

void OrByte::SFML::pushRegion(const Rect& pixelRect) const
{
    this->_regions.push_back(pixelRect); this->applyRegionView();
}

void OrByte::SFML::popRegion() const
{
    if (!this->_regions.empty())
        this->_regions.pop_back();
    this->applyRegionView();
}

Vector2u OrByte::SFML::referenceSize() const
{
    return this->regionSize();
}

Vector2 OrByte::SFML::currentRatio() const
{
    if (!this->_regions.empty() && !this->_gameMode)
        return { 1.f, 1.f };
    const Vector2u size = this->referenceSize();
    return getWindowRatio(sf::Vector2u(size.x, size.y));
}

Vector2 OrByte::SFML::currentOffset() const
{
    if (!this->_regions.empty() && !this->_gameMode)
        return { 0.f, 0.f };
    const Vector2u size = this->referenceSize();
    return getWindowOffset(sf::Vector2u(size.x, size.y));
}

void OrByte::SFML::setCursor(CursorType type)
{
    if (!this->_isOpen || !this->_window || type == this->_currentCursor)
        return;

    const sf::Cursor* cursor = nullptr;

    switch (type) {
        case CursorType::ResizeHorizontal:
            cursor = this->_cursorHorizontal ? &*this->_cursorHorizontal : nullptr;
            break;
        case CursorType::ResizeVertical:
            cursor = this->_cursorVertical ? &*this->_cursorVertical : nullptr;
            break;
        case CursorType::Move:
            cursor = this->_cursorMove ? &*this->_cursorMove : nullptr;
            break;
        case CursorType::Click:
            cursor = this->_cursorClick ? &*this->_cursorClick : nullptr;
            break;
        case CursorType::ResizeDiagonalNWSE:
            cursor = this->_cursorDiagonalNWSE ? &*this->_cursorDiagonalNWSE : nullptr;
            break;
        case CursorType::ResizeDiagonalNESW:
            cursor = this->_cursorDiagonalNESW ? &*this->_cursorDiagonalNESW : nullptr;
            break;
        default:
            cursor = this->_cursorArrow ? &*this->_cursorArrow : nullptr;
            break;
    }

    if (!cursor)
        cursor = this->_cursorArrow ? &*this->_cursorArrow : nullptr;

    if (cursor) {
        this->_window->setMouseCursor(*cursor);
        this->_currentCursor = type;
    }
}

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::GRAPHICAL_LIB;
    }

    OrByte::IGraphic *factory()
    {
        return new OrByte::SFML();
    }
}
