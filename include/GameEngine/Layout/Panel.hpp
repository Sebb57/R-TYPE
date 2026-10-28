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
    void draw(const IGraphic& graphicLib, Vector2u size) override;
};

} // namespace OrByte

#endif /* PANEL_HPP_ */