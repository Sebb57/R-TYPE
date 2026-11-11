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
#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

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

// only callable at compile time
namespace {

template <auto V>
consteval std::string_view rawName() { return __PRETTY_FUNCTION__; }

template <auto V>
consteval std::string_view enumName()
{
    std::string_view s = rawName<V>();
    s = s.substr(s.find("V = ") + 4);
    s = s.substr(0, s.find_first_of(";]"));
    if (const auto pos = s.rfind("::"); pos != std::string_view::npos)
        s = s.substr(pos + 2);
    return s;
}

template <typename E, std::size_t... I>
consteval auto makeEnumNames(std::index_sequence<I...> /*s*/) { return std::array<std::string_view, sizeof...(I)>{enumName<static_cast<E>(I)>()...}; }

} // namespace 

template <typename E> requires std::is_enum_v<E> && requires { E::ENUM_END; }
[[nodiscard]] constexpr std::string_view enumToString(E value)
{
    constexpr auto count = static_cast<std::size_t>(E::ENUM_END);
    static constexpr auto names = makeEnumNames<E>(std::make_index_sequence<count>{});

    const auto index = static_cast<std::size_t>(value);
    return index < count ? names[index] : std::string_view{};
}

#endif /* UTILS_HPP_ */
