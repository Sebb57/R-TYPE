/**
 * @file SFML.hpp
 * @brief SFML Header
 *
 * @author SkyReox
 * @date 2026-10-12
**/

#pragma once
#ifndef SFML_HPP_
    #define SFML_HPP_

    #include "plugins/AGraphic.hpp"
    #include <chrono>
    #include <filesystem>
    #include <map>
    #include <memory>
    #include <queue>
    #include <string>
    #include <unordered_map>
    #include <SFML/Graphics.hpp>
    #include <SFML/System.hpp>
    #include <SFML/Window.hpp>

namespace OrByte {
    class SFML : public AGraphic {
        std::chrono::steady_clock::time_point _nextFrameTime = std::chrono::steady_clock::now();
        std::unique_ptr<sf::RenderWindow> _window;
        mutable std::unordered_map<std::string, sf::Font> _fonts;
        mutable std::unordered_map<std::string, sf::Texture> _textures;

        const sf::Font *getFont(const std::filesystem::path& path) const;
        const sf::Texture *getTexture(const std::filesystem::path& path) const;

        public:
            SFML() = default;
            ~SFML() override = default;

            void open(const std::size_t width, const std::size_t height, std::string title) final;
            void close() final;
            std::queue<Event> listen() const final;
            void clear() const final;
            void update() final;

            virtual void drawRectangle(const Transform& transform, Vector2 size, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor) const final;
            virtual void drawCircle(const Transform& transform, unsigned radius, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor) const final;
            virtual void drawImage(const Transform& transform, Vector2 size, std::filesystem::path path) const final;
            virtual void drawText(const Transform& transform, std::string text, unsigned fontSize, RGBA fillColor, unsigned outlineThickness, RGBA outlineColor, std::filesystem::path font) const final;

            std::pair<unsigned, unsigned> getWindowSize() const noexcept final;
    };
}

#endif /* SFML_HPP_ */
