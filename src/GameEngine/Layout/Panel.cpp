/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Panel.cpp
*/

#include "GameEngine/Layout/Panel.hpp"

void OrByte::Panel::draw(const IGraphic& graphicLib, Vector2u size)
{
    const float headerHeight = static_cast<float>(this->getHeaderHeight());
    const float w = static_cast<float>(size.x);
    const float h = static_cast<float>(size.y);

    graphicLib.drawRectangle(Transform{ { w/2, h/2 }, 0.f }, { w, h }, RGBA{ 40, 40, 40, 255 }, 1, RGBA{ 80, 80, 80, 255 });

    if (headerHeight <= 0.f)
        return;
    graphicLib.drawRectangle(Transform{ { w/2, headerHeight/2 }, 0.f }, { w, headerHeight }, RGBA{ 60, 60, 60, 255 }, 0, RGBA{ 0, 0, 0, 0 });
    graphicLib.drawText(Transform{ { 6.f, 3.f }, 0.f }, this->getTitle(), 14, 0.f, RGBA{ 220, 220, 220, 255 }, 0, RGBA{ 0, 0, 0, 0 }, OrByte::constants::FONT_DEFAULT);
}
