/**
 * @file Theme.hpp
 * @brief shared colors and layout of the client screens, in server coordinates
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_THEME_HPP
#define CLIENT_SCENES_THEME_HPP

#include <array>
#include "constants.hpp"
#include "plugins/IGraphic.hpp"

namespace client::theme {

inline constexpr float Width = OrByte::constants::SERVER_WINDOW_X;
inline constexpr float Height = OrByte::constants::SERVER_WINDOW_Y;
inline constexpr float Margin = 160.0f;
inline constexpr float Column = 1100.0f;

inline constexpr OrByte::RGBA Background(13, 18, 29);
inline constexpr OrByte::RGBA Panel(18, 25, 38);
inline constexpr OrByte::RGBA PanelOutline(39, 52, 72);
inline constexpr OrByte::RGBA Row(23, 32, 47);
inline constexpr OrByte::RGBA RowSelected(34, 54, 81);
inline constexpr OrByte::RGBA RowOutline(71, 111, 166);
inline constexpr OrByte::RGBA Accent(111, 190, 255);
inline constexpr OrByte::RGBA Text(225, 232, 245);
inline constexpr OrByte::RGBA Muted(129, 150, 182);
inline constexpr OrByte::RGBA Disabled(80, 92, 110);
inline constexpr OrByte::RGBA Ready(117, 216, 168);
inline constexpr OrByte::RGBA Waiting(230, 180, 105);
inline constexpr std::array<OrByte::RGBA, 4> Slots({(111, 190, 255), {255, 170, 90}, {117, 216, 168}, {240, 120, 180))};

} // namespace client::theme

#endif // CLIENT_SCENES_THEME_HPP
