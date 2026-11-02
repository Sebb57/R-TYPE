/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Utils.hpp
*/

#pragma once
#ifndef UTILS_HPP_
    #define UTILS_HPP_

#include <algorithm>

struct Vector2
{
    float x;
    float y;

    Vector2(float x = 0.0F, float y = 0.0F) : x(x), y(y) {}
};

template <typename S>
void serialize(S& s, Vector2& vector)
{
    s.value4b(vector.x);
    s.value4b(vector.y);
}

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

[[nodiscard]] inline float clampToBound(float center, float halfSize, float bound)
{
    if (bound <= 0.0F)
        return center;
    if (halfSize * 2.0F >= bound)
        return bound / 2.0F;
    return std::clamp(center, halfSize, bound - halfSize);
}

#endif /* UTILS_HPP_ */
