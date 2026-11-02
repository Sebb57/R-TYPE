/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Panel.hpp
*/

#pragma once
    #ifndef PANEL_HPP_
    #define PANEL_HPP_

#include "constants.hpp"
#include "GameEngine/Layout/Widget.hpp"
#include <string>
#include <utility>

namespace OrByte {

class Panel : public Widget {
    public:
        Panel(std::string title, int size) : Widget(std::move(title), size) {}

    protected:
        static constexpr int TITLE_HEIGHT = 24;

        void draw(const IGraphic& graphicLib, Vector2u size) override;

        [[nodiscard]] int getHeaderHeight() const override { return this->getTitle().empty() ? 0 : TITLE_HEIGHT; }
};

} // namespace OrByte

#endif /* PANEL_HPP_ */