/*
** EPITECH PROJECT, 2026
** RType
** File description:
** SceneView.cpp
*/

#include "GameEngine/Layout/SceneView.hpp"

void OrByte::SceneView::draw(const OrByte::IGraphic& graphicLib, Vector2u size)
{
    const float w = static_cast<float>(size.x);
    const float h = static_cast<float>(size.y);

    graphicLib.drawRectangle(Transform( { w / 2, h / 2 }, 0.f ), { w, h }, RGBA{ 0, 0, 0, 255 }, 0, RGBA{ 0, 0, 0, 0 });

    graphicLib.setGameMode(true);
    if (this->_render)
        this->_render(graphicLib);
    graphicLib.setGameMode(false);
}

bool OrByte::SceneView::onEvent(const OrByte::Event& event, Vector2 local)
{
    if (!this->_onGameEvent)
        return false;
    OrByte::Event e = event;

    if (event.type == OrByte::Event::Type::MouseMoved ||
        event.type == OrByte::Event::Type::MouseButtonPressed ||
        event.type == OrByte::Event::Type::MouseButtonReleased) {
        const float cw = static_cast<float>(getContentRect().w);
        const float ch = static_cast<float>(getContentRect().h);
        const float ratio = std::min(cw / OrByte::constants::SERVER_WINDOW_X, ch / OrByte::constants::SERVER_WINDOW_Y);
        const float offX = (cw - OrByte::constants::SERVER_WINDOW_X * ratio) / 2;
        const float offY = (ch - OrByte::constants::SERVER_WINDOW_Y * ratio) / 2;

        e.mouseX = static_cast<int>((local.x - offX) / ratio);
        e.mouseY = static_cast<int>((local.y - offY) / ratio);
    }
    return this->_onGameEvent(e);
}
