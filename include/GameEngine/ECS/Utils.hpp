/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Utils.hpp
*/

#pragma once
#ifndef UTILS_HPP_
    #define UTILS_HPP_

struct Vector2
{
    float x;
    float y;

    Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct Vector2u
{
    unsigned x;
    unsigned y;

    Vector2u(unsigned x = 0, unsigned y = 0) : x(x), y(y) {}
};

struct Rect {
    int x{0};
    int y{0};
    int w{0};
    int h{0};

    Rect() = default;

    Rect(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h)
    {}

    [[nodiscard]] bool contains(int px, int py) const
    {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

#endif /* UTILS_HPP_ */
