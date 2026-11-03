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
    this->_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height, 32), title, sf::Style::Default);
    this->_window->setFramerateLimit(constants::FPS_LIMIT);

    this->_width = width;
    this->_height = height;
    this->_nextFrameTime = std::chrono::steady_clock::now();

    this->_cursorArrow.loadFromSystem(sf::Cursor::Arrow);
    this->_cursorHorizontal.loadFromSystem(sf::Cursor::SizeHorizontal);
    this->_cursorVertical.loadFromSystem(sf::Cursor::SizeVertical);
    this->_cursorMove.loadFromSystem(sf::Cursor::SizeAll);
    this->_currentCursor = CursorType::Arrow;

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
    sf::Event sfmlEvent;

    if (!this->_isOpen || !this->_window)
        return events;
    while (this->_window->pollEvent(sfmlEvent)) {
        Event event;

        if (sfmlEvent.type == sf::Event::Closed) {
            event.type = Event::Type::Closed;
            events.push(event);
            continue;
        }
        if (sfmlEvent.type == sf::Event::Resized) {
            this->_window->setView(sf::View(sf::FloatRect(0, 0, sfmlEvent.size.width, sfmlEvent.size.height)));
            event.type = Event::Type::Resized;
            event.width = sfmlEvent.size.width;
            event.height = sfmlEvent.size.height;
            events.push(event);
            continue;
        }
        if (sfmlEvent.type == sf::Event::KeyPressed || sfmlEvent.type == sf::Event::KeyReleased || sfmlEvent.type == sf::Event::TextEntered) {
            switch (sfmlEvent.key.code) {
                case sf::Keyboard::Left:
                    event.key = Event::Key::Left;
                    break;
                case sf::Keyboard::Right:
                    event.key = Event::Key::Right;
                    break;
                case sf::Keyboard::Down:
                    event.key = Event::Key::Down;
                    break;
                case sf::Keyboard::Up:
                    event.key = Event::Key::Up;
                    break;
                case sf::Keyboard::Enter:
                    event.key = Event::Key::Enter;
                    break;
                case sf::Keyboard::Escape:
                    event.key = Event::Key::Escape;
                    break;
                case sf::Keyboard::Space:
                    event.key = Event::Key::Space;
                    break;
                case sf::Keyboard::Backspace:
                    event.key = Event::Key::Backspace;
                    break;
                case sf::Keyboard::Delete:
                    event.key = Event::Key::Delete;
                    break;
                case sf::Keyboard::Insert:
                    event.key = Event::Key::Insert;
                    break;
                case sf::Keyboard::A:
                    event.key = Event::Key::A;
                    break;
                case sf::Keyboard::B:
                    event.key = Event::Key::B;
                    break;
                case sf::Keyboard::C:
                    event.key = Event::Key::C;
                    break;
                case sf::Keyboard::D:
                    event.key = Event::Key::D;
                    break;
                case sf::Keyboard::E:
                    event.key = Event::Key::E;
                    break;
                case sf::Keyboard::F:
                    event.key = Event::Key::F;
                    break;
                case sf::Keyboard::G:
                    event.key = Event::Key::G;
                    break;
                case sf::Keyboard::H:
                    event.key = Event::Key::H;
                    break;
                case sf::Keyboard::I:
                    event.key = Event::Key::I;
                    break;
                case sf::Keyboard::J:
                    event.key = Event::Key::J;
                    break;
                case sf::Keyboard::K:
                    event.key = Event::Key::K;
                    break;
                case sf::Keyboard::L:
                    event.key = Event::Key::L;
                    break;
                case sf::Keyboard::M:
                    event.key = Event::Key::M;
                    break;
                case sf::Keyboard::N:
                    event.key = Event::Key::N;
                    break;
                case sf::Keyboard::O:
                    event.key = Event::Key::O;
                    break;
                case sf::Keyboard::P:
                    event.key = Event::Key::P;
                    break;
                case sf::Keyboard::Q:
                    event.key = Event::Key::Q;
                    break;
                case sf::Keyboard::R:
                    event.key = Event::Key::R;
                    break;
                case sf::Keyboard::S:
                    event.key = Event::Key::S;
                    break;
                case sf::Keyboard::T:
                    event.key = Event::Key::T;
                    break;
                case sf::Keyboard::U:
                    event.key = Event::Key::U;
                    break;
                case sf::Keyboard::V:
                    event.key = Event::Key::V;
                    break;
                case sf::Keyboard::W:
                    event.key = Event::Key::W;
                    break;
                case sf::Keyboard::X:
                    event.key = Event::Key::X;
                    break;
                case sf::Keyboard::Y:
                    event.key = Event::Key::Y;
                    break;
                case sf::Keyboard::Z:
                    event.key = Event::Key::Z;
                    break;
                default:
                    break;
            }
        }
        if (sfmlEvent.type == sf::Event::KeyPressed) {
            event.type = Event::Type::KeyPressed;
            event.keyChar = static_cast<unsigned>(sfmlEvent.text.unicode);
            events.push(event);
        }
        if (sfmlEvent.type == sf::Event::KeyReleased) {
            event.type = Event::Type::KeyReleased;
            event.keyChar = static_cast<unsigned>(sfmlEvent.text.unicode);
            events.push(event);
        }
        if (sfmlEvent.type == sf::Event::TextEntered) {
            event.type = Event::Type::TextEntered;
            event.keyChar = static_cast<char>(sfmlEvent.text.unicode);
            events.push(event);
        }
        if (sfmlEvent.type == sf::Event::MouseButtonPressed) {
            event.type = Event::Type::MouseButtonPressed;
            event.windowX = sfmlEvent.mouseButton.x;
            event.windowY = sfmlEvent.mouseButton.y;
            Vector2 mousePos = getPositionFromWindow({ event.windowX, event.windowY }, this->_window->getSize());
            event.mouseX = static_cast<int>(mousePos.x);
            event.mouseY = static_cast<int>(mousePos.y);
            switch (sfmlEvent.mouseButton.button) {
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
                events.push(event);
        }
        if (sfmlEvent.type == sf::Event::MouseButtonReleased) {
            event.type = Event::Type::MouseButtonReleased;
            event.windowX = sfmlEvent.mouseButton.x;
            event.windowY = sfmlEvent.mouseButton.y;
            Vector2 mousePos = getPositionFromWindow({ event.windowX, event.windowY }, this->_window->getSize());
            event.mouseX = static_cast<int>(mousePos.x);
            event.mouseY = static_cast<int>(mousePos.y);
            switch (sfmlEvent.mouseButton.button) {
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
            events.push(event);
        }
        if (sfmlEvent.type == sf::Event::MouseMoved) {
            event.type = Event::Type::MouseMoved;
            event.windowX = sfmlEvent.mouseMove.x;
            event.windowY = sfmlEvent.mouseMove.y;
            Vector2 mousePos = getPositionFromWindow({ event.windowX, event.windowY }, this->_window->getSize());
            event.mouseX = static_cast<int>(mousePos.x);
            event.mouseY = static_cast<int>(mousePos.y);
            events.push(event);
        }
        if (sfmlEvent.type == sf::Event::MouseWheelScrolled) {
            event.type = Event::Type::MouseWheelScrolled;
            event.windowX = sfmlEvent.mouseWheelScroll.x;
            event.windowY = sfmlEvent.mouseWheelScroll.y;
            event.wheelDelta = sfmlEvent.mouseWheelScroll.delta;
            Vector2 mousePos = getPositionFromWindow({ event.windowX, event.windowY }, this->_window->getSize());
            event.mouseX = static_cast<int>(mousePos.x);
            event.mouseY = static_cast<int>(mousePos.y);
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
    rectangle.setRotation(transform.rotation);
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
    circle.setRotation(transform.rotation);
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
        if (!loaded.loadFromFile(path.string()))
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

void OrByte::SFML::drawImage(const Transform& transform, Vector2 size, std::filesystem::path path, bool contain) const
{
    if (!this->_isOpen || !this->_window)
        return;
    const sf::Texture *texture = this->getTexture(path);
    if (texture == nullptr)
        return;

    const sf::Vector2u textureSize = texture->getSize();
    if (textureSize.x == 0 || textureSize.y == 0)
        return;

    sf::Sprite sprite;
    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();
    const float scaleX = (size.x * ratio.x) / static_cast<float>(textureSize.x);
    const float scaleY = (size.y * ratio.y) / static_cast<float>(textureSize.y);
    const float scale = contain ? std::min(scaleX, scaleY) : scaleY;

    sprite.setTexture(*texture);
    sprite.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x,
                                    offset.y + transform.position.y * ratio.y));
    sprite.setScale({ scale, scale });
    sprite.setOrigin({ textureSize.x / 2.f, textureSize.y / 2.f });
    sprite.setRotation(transform.rotation);

    this->_window->draw(sprite);
}

void OrByte::SFML::drawImage(const Transform& transform, Vector2 size, std::filesystem::path path, Rect rect, bool contain) const
{
    if (!this->_isOpen || !this->_window)
        return;
    const sf::Texture *texture = this->getTexture(path);
    if (texture == nullptr)
        return;

    if (rect.w <= 0 || rect.h <= 0)
        return;

    sf::Sprite sprite;
    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();
    const float scaleX = (size.x * ratio.x) / static_cast<float>(rect.w);
    const float scaleY = (size.y * ratio.y) / static_cast<float>(rect.h);
    const float scale = contain ? std::min(scaleX, scaleY) : scaleY;

    sprite.setTexture(*texture);
    sprite.setTextureRect(sf::IntRect(rect.x, rect.y, rect.w, rect.h));
    sprite.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x,
                                    offset.y + transform.position.y * ratio.y));
    sprite.setScale({ scale, scale });
    sprite.setOrigin({ rect.w / 2.f, rect.h / 2.f });
    sprite.setRotation(transform.rotation);

    this->_window->draw(sprite);
}

Vector2 OrByte::SFML::drawText(const Transform& transform, std::string text, unsigned fontSize, float maxWidth, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor, std::filesystem::path font) const
{
    if (!this->_isOpen || !this->_window)
        return {0.f, 0.f};
    const sf::Font *sfmlFont = this->getFont(font);
    if (sfmlFont == nullptr)
        return {0.f, 0.f};

    sf::Text sfmlText;
    const Vector2 ratio = this->currentRatio();
    const Vector2 offset = this->currentOffset();

    sfmlText.setFont(*sfmlFont);
    sfmlText.setCharacterSize(static_cast<unsigned>(static_cast<float>(fontSize) * ratio.y));
    sfmlText.setFillColor(colorNormalize(fillColor));
    sfmlText.setOutlineThickness(outlineThickness);
    sfmlText.setOutlineColor(colorNormalize(outlineColor));
    sfmlText.setString(text);

    if (maxWidth > 0.f) {
        const float limit = maxWidth * ratio.x;

        if (sfmlText.getLocalBounds().width > limit) {
            const std::string ellipsis = "...";
            while (!text.empty()) {
                text.pop_back();
                sfmlText.setString(text + ellipsis);
                if (sfmlText.getLocalBounds().width <= limit)
                    break;
            }
            if (text.empty())
                sfmlText.setString(ellipsis);
        }
    }

    sfmlText.setPosition(sf::Vector2f(offset.x + transform.position.x * ratio.x, offset.y + transform.position.y * ratio.y));
    this->_window->draw(sfmlText);

    const auto localBounds = sfmlText.getLocalBounds();
    return {localBounds.width, localBounds.height};
}

Vector2 OrByte::SFML::getTextSize(std::string text, unsigned fontSize, float maxWidth, std::filesystem::path font) const
{
    if (!this->_isOpen || !this->_window)
        return {0.f, 0.f};
    const sf::Font *sfmlFont = this->getFont(font);
    if (sfmlFont == nullptr)
        return {0.f, 0.f};

    sf::Text sfmlText;
    const Vector2 ratio = this->currentRatio();

    sfmlText.setFont(*sfmlFont);
    sfmlText.setCharacterSize(static_cast<unsigned>(static_cast<float>(fontSize) * ratio.y));
    sfmlText.setString(text);

    if (maxWidth > 0.f) {
        const float limit = maxWidth * ratio.x;

        if (sfmlText.getLocalBounds().width > limit) {
            const std::string ellipsis = "...";
            while (!text.empty()) {
                text.pop_back();
                sfmlText.setString(text + ellipsis);
                if (sfmlText.getLocalBounds().width <= limit)
                    break;
            }
            if (text.empty())
                sfmlText.setString(ellipsis);
        }
    }

    const auto localBounds = sfmlText.getLocalBounds();
    return {localBounds.width, localBounds.height};
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
        this->_window->setView(sf::View(sf::FloatRect(0, 0, windowSize.x, windowSize.y)));
        return;
    }

    const Rect& rect = this->_regions.back();
    sf::View view(sf::FloatRect(0, 0, rect.w, rect.h));

    view.setViewport(sf::FloatRect(rect.x / windowSize.x, rect.y / windowSize.y, rect.w / windowSize.x, rect.h / windowSize.y));
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

    switch (type) {
        case CursorType::ResizeHorizontal:
            this->_window->setMouseCursor(this->_cursorHorizontal);
            break;
        case CursorType::ResizeVertical:
            this->_window->setMouseCursor(this->_cursorVertical);
            break;
        case CursorType::Move:
            this->_window->setMouseCursor(this->_cursorMove);
            break;
        default:
            this->_window->setMouseCursor(this->_cursorArrow);
            break;
    }
    this->_currentCursor = type;
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
