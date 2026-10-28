/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Panel.cpp
*/

#include "GameEngine/Layout/Panel.hpp"

void OrByte::Panel::draw(const IGraphic& graphicLib, Vector2u size)
{
    constexpr float titleHeight = 24.f;
    const float w = static_cast<float>(size.x);
    const float h = static_cast<float>(size.y);

    graphicLib.drawRectangle(Transform{ { w/2, h/2 }, 0.f }, { w, h }, RGBA{ 40, 40, 40, 255 }, 1, RGBA{ 80, 80, 80, 255 });

    if (this->getTitle().empty())
        return;
    graphicLib.drawRectangle(Transform{ { w/2, titleHeight/2 }, 0.f }, { w, titleHeight }, RGBA{ 60, 60, 60, 255 }, 0, RGBA{ 0, 0, 0, 0 });
    graphicLib.drawText(Transform{ { 6.f, 3.f }, 0.f }, this->getTitle(), 14, RGBA{ 220, 220, 220, 255 }, 0, RGBA{ 0, 0, 0, 0 }, OrByte::constants::FONT_DEFAULT);
}
