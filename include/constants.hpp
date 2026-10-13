/**
 * @file constants.hpp
 * @brief r-type constants
 *
 * @author Seb
 * @date 2026-10-05
**/

#pragma once
#ifndef CONSTANTS_HPP_
    #define CONSTANTS_HPP_

namespace OrByte::constants {

inline constexpr int SUCCESS = 0;
inline constexpr int FAILURE = 84;

inline constexpr int FPS_LIMIT = 60;
inline constexpr int SERVER_WINDOW_X = 1920;
inline constexpr int SERVER_WINDOW_Y = 1080;

inline constexpr const char *FONT_DEFAULT = "assets/fonts/NotoSans-Regular.ttf";
inline constexpr const char *SPRITE_PLAYER = "assets/sprites/thanks.png";
inline constexpr const char *SPRITE_PROJECTILE = "assets/sprites/thanks.png";

inline constexpr float PLAYER_WIDTH = 130.0f;
inline constexpr float PLAYER_HEIGHT = 100.0f;
inline constexpr float PLAYER_SPEED = 600.0f;
inline constexpr float PROJECTILE_WIDTH = 52.0f;
inline constexpr float PROJECTILE_HEIGHT = 40.0f;
inline constexpr float PROJECTILE_SPEED = 1400.0f;
inline constexpr double FIRE_RATE = 6.0;

};

#endif /* CONSTANTS_HPP_ */

