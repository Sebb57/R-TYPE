/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Widget.hpp
*/

#pragma once
#ifndef WIDGET_HPP_
    #define WIDGET_HPP_

#include <string>
#include "GameEngine/ECS/Utils.hpp"

namespace OrByte {

// class IWidget {
//     public:
//         virtual ~IWidget() = default;
//         virtual std::string title() const = 0;
//         virtual void draw(sf::RenderTarget& target, sf::Vector2f size) = 0; // coords locales
//         virtual bool onEvent(const sf::Event&, sf::Vector2f localMouse) { return false; }
//         virtual sf::Vector2f minSize() const { return {120.f, 80.f}; }
// };

// void drawClipped(sf::RenderTarget& target, IWidget& w, Rect r) {
//     sf::Vector2f ts(target.getSize());
//     sf::View v(sf::FloatRect(0.f, 0.f, r.w, r.h));
//     v.setViewport(sf::FloatRect(r.x / ts.x, r.y / ts.y, r.w / ts.x, r.h / ts.y));
//     target.setView(v);
//     w.draw(target, {r.w, r.h});
//     target.setView(target.getDefaultView());
// }

}

#endif /* WIDGET_HPP_ */
