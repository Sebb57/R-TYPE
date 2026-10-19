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

struct Rect {
    int x{0};
    int y{0};
    int width{0};
    int height{0};

    Rect() = default;

    Rect(int x, int y, int width, int height)
        : x(x), y(y), width(width), height(height)
    {}
};

#endif /* UTILS_HPP_ */
