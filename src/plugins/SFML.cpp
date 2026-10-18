/**
 * @file SFML.cpp
 * @brief SFML
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#include "constants.hpp"
#include "GameEngine/ECS/Utils.hpp"
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

    sf::Vector2f getPositionFromRatio(const Vector2 position, const sf::Vector2u windowSize)
    {
        const Vector2 windowRatio = getWindowRatio(windowSize);
        const Vector2 offset = getWindowOffset(windowSize);

        return { offset.x + position.x * windowRatio.x, offset.y + position.y * windowRatio.y };
    }

    Vector2 getPositionFromWindow(const sf::Vector2i position, const sf::Vector2u windowSize)
    {
        const Vector2 windowRatio = getWindowRatio(windowSize);
        const Vector2 offset = getWindowOffset(windowSize);

        return { (static_cast<float>(position.x) - offset.x) / windowRatio.x, (static_cast<float>(position.y) - offset.y) / windowRatio.y };
    }

    sf::Vector2f getSizeFromRatio(const Vector2 size, const Vector2 windowRatio)
    {
        return { size.x * windowRatio.y, size.y * windowRatio.y };
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
            Vector2 mousePos = getPositionFromWindow(sf::Mouse::getPosition(*this->_window), this->_window->getSize());
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
            Vector2 mousePos = getPositionFromWindow(sf::Mouse::getPosition(*this->_window), this->_window->getSize());
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
            Vector2 mousePos = getPositionFromWindow(sf::Mouse::getPosition(*this->_window), this->_window->getSize());
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
    Vector2 windowRatio = getWindowRatio(this->_window->getSize());
    sf::Vector2f sizeRatio = getSizeFromRatio(size, windowRatio);

    rectangle.setPosition(getPositionFromRatio(transform.position, this->_window->getSize()));
    rectangle.setSize(sizeRatio);
    rectangle.setOrigin({ sizeRatio.x / 2, sizeRatio.y / 2 });
    rectangle.setRotation(transform.rotation);
    rectangle.setFillColor(colorNormalize(fillColor));
    rectangle.setOutlineThickness(outlineThickness);
    rectangle.setOutlineColor(colorNormalize(outlineColor));

    this->_window->draw(rectangle);
}

void OrByte::SFML::drawCircle(const Transform& transform, unsigned radius, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor) const
{
    sf::CircleShape circle;

    if (!this->_isOpen || !this->_window)
        return;
    Vector2 windowRatio = getWindowRatio(this->_window->getSize());
    float radiusRatio = radius * windowRatio.y;

    circle.setPosition(getPositionFromRatio(transform.position, this->_window->getSize()));
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

void OrByte::SFML::drawImage(const Transform& transform, Vector2 size, std::filesystem::path path) const
{
    if (!this->_isOpen || !this->_window)
        return;
    const sf::Texture *texture = this->getTexture(path);
    if (texture == nullptr)
        return;

    sf::Sprite sprite;
    Vector2 windowRatio = getWindowRatio(this->_window->getSize());
    sf::Vector2u textureSize = texture->getSize();
    float scale = getSizeFromRatio(size, windowRatio).y / textureSize.y;

    sprite.setPosition(getPositionFromRatio(transform.position, this->_window->getSize()));
    sprite.setScale({scale, scale});
    sprite.setTexture(*texture);
    sprite.setOrigin({textureSize.x / 2.0f, textureSize.y / 2.0f});
    sprite.setRotation(transform.rotation);

    this->_window->draw(sprite);
}

void OrByte::SFML::drawImage(const Transform& transform, Vector2 size, std::filesystem::path path, Rect rect) const
{
    if (!this->_isOpen || !this->_window)
        return;
    const sf::Texture *texture = this->getTexture(path);
    if (texture == nullptr)
        return;

    sf::Sprite sprite;
    sf::IntRect sfmlRect(rect.x, rect.y, rect.width, rect.height);
    Vector2 windowRatio = getWindowRatio(this->_window->getSize());
    sf::Vector2u textureSize = texture->getSize();
    float scale = getSizeFromRatio(size, windowRatio).y / rect.height;

    sprite.setPosition(getPositionFromRatio(transform.position, this->_window->getSize()));
    sprite.setScale({scale, scale});
    sprite.setTexture(*texture);
    sprite.setTextureRect(sfmlRect);
    sprite.setOrigin({rect.width / 2.0f, rect.height / 2.0f});
    sprite.setRotation(transform.rotation);

    this->_window->draw(sprite);
}

void OrByte::SFML::drawText(const Transform& transform, std::string text, unsigned fontSize, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor, std::filesystem::path font) const
{
    if (!this->_isOpen || !this->_window)
        return;
    const sf::Font *sfmlFont = this->getFont(font);
    if (sfmlFont == nullptr)
        return;

    sf::Text sfmlText;
    Vector2 windowRatio = getWindowRatio(this->_window->getSize());

    sfmlText.setFont(*sfmlFont);
    sfmlText.setCharacterSize(fontSize * windowRatio.y);
    sfmlText.setFillColor(colorNormalize(fillColor));
    sfmlText.setOutlineThickness(outlineThickness);
    sfmlText.setOutlineColor(colorNormalize(outlineColor));
    sfmlText.setString(text);

    sfmlText.setPosition(getPositionFromRatio(transform.position, this->_window->getSize()));
    this->_window->draw(sfmlText);
}

std::pair<unsigned, unsigned> OrByte::SFML::getWindowSize() const noexcept
{
    sf::Vector2u windowSize = this->_window->getSize();

    return { windowSize.x, windowSize.y };
}

extern "C" {
    OrByte::IGraphic *factory()
    {
        return new OrByte::SFML();
    }
}
