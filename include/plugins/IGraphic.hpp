/**
 * @file IGraphic.hpp
 * @brief Graphical Library Interface
 *
 * @author SkyReox
 * @date 2026-09-15
**/

#ifndef IGRAPHIC_HPP
    #define IGRAPHIC_HPP

    #include "constants.hpp"
    #include "GameEngine/ECS/Components/Transform.hpp"
    #include "GameEngine/ECS/Utils.hpp"
    #include "GameEngine/Event.hpp"
    #include <cstddef>
    #include <cstdint>
    #include <filesystem>
    #include <memory>
    #include <queue>
    #include <vector>
    #include <string>

enum {
    FILL_PADDING = 1
};

namespace OrByte {
    struct RGBA {
        std::uint8_t r = 0;
        std::uint8_t g = 0;
        std::uint8_t b = 0;
        std::uint8_t a = 255;
    };

    class IGraphic {
        public:
            virtual void open(std::size_t width = 1920, std::size_t height = 1080, std::string title = "LayoutEngine") = 0;
            virtual void close() = 0;
            [[nodiscard]] virtual std::queue<Event> listen() const = 0;
            virtual void clear() const = 0;
            virtual void update() = 0;
            [[nodiscard]] virtual bool isOpen() const noexcept = 0;

            virtual void drawRectangle(const Transform& transform, Vector2 size, RGBA fillColor = {255, 255, 255}, unsigned outlineThickness = 0, RGBA outlineColor = {0, 0, 0}) const = 0;
            virtual void drawCircle(const Transform& transform, unsigned radius, RGBA fillColor = {255, 255, 255}, unsigned outlineThickness = 0, RGBA outlineColor = {0, 0, 0}) const = 0;
            virtual void drawImage(const Transform& transform, Vector2 size, std::filesystem::path path) const = 0;
            virtual void drawText(const Transform& transform, std::string text, unsigned fontSize = 20, RGBA fillColor = {255, 255, 255}, unsigned outlineThickness = 0, RGBA outlineColor = {0, 0, 0}, std::filesystem::path font = constants::FONT_DEFAULT) const = 0;

            [[nodiscard]] virtual std::pair<unsigned, unsigned> getWindowSize() const noexcept = 0;

            IGraphic& operator=(const IGraphic& object) = delete;
            IGraphic& operator=(IGraphic&& object) = delete;

            IGraphic() = default;
            IGraphic(const IGraphic& object) = delete;
            IGraphic(IGraphic&& object) = delete;

            virtual ~IGraphic() = default;
    };

} // namespace Layout
#endif // IGRAPHIC_HPP
