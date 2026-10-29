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

    #include <array>

namespace OrByte::constants {

inline constexpr int SUCCESS = 0;
inline constexpr int FAILURE = 84;

inline constexpr int FPS_LIMIT = 60;
inline constexpr int SERVER_WINDOW_X = 1920;
inline constexpr int SERVER_WINDOW_Y = 1080;

inline constexpr const char *CONFIGS_DIR = "./configs";
inline constexpr const char *FONT_DEFAULT = "assets/fonts/NotoSans-Regular.ttf";
inline constexpr const char *SPRITE_PLAYER = "./assets/sprites/thanks.png";
inline constexpr const char *SPRITE_PROJECTILE = "./assets/sprites/thanks.png";

inline constexpr std::array<float, 4> PLAYER_FRAME_WIDTHS{263.0f, 262.0f, 263.0f, 263.0f};
inline constexpr std::array<float, 4> PLAYER_FRAME_HEIGHTS{116.0f, 113.0f, 123.0f, 124.0f};
inline constexpr float PLAYER_HEIGHT = 60.0f;
inline constexpr float ENEMY_FRAME_WIDTH = 27.0f;
inline constexpr float ENEMY_FRAME_HEIGHT = 30.0f;
inline constexpr float ENEMY_HEIGHT = 84.0f;
inline constexpr float ENEMY_WIDTH = ENEMY_HEIGHT * ENEMY_FRAME_WIDTH / ENEMY_FRAME_HEIGHT;
inline constexpr float ENEMY_COLLIDER_WIDTH = 44.0f;
inline constexpr float ENEMY_COLLIDER_HEIGHT = 44.0f;
inline constexpr float BONUS_FRAME_SIZE = 160.0f;
inline constexpr float BONUS_WIDTH = 64.0f;
inline constexpr float BONUS_HEIGHT = 64.0f;
inline constexpr float BONUS_COLLIDER_WIDTH = 44.0f;
inline constexpr float BONUS_COLLIDER_HEIGHT = 44.0f;

inline constexpr float playerWidthForSlot(unsigned slot)
{
    const unsigned index = slot < PLAYER_FRAME_WIDTHS.size() ? slot : 0;
    return PLAYER_HEIGHT * PLAYER_FRAME_WIDTHS[index] / PLAYER_FRAME_HEIGHTS[index];
}
inline constexpr float PLAYER_SPEED = 600.0f;
inline constexpr float PROJECTILE_FRAME_WIDTH = 19.0f;
inline constexpr float PROJECTILE_FRAME_HEIGHT = 6.0f;
inline constexpr float PROJECTILE_WIDTH = 47.5f;
inline constexpr float PROJECTILE_HEIGHT = PROJECTILE_WIDTH * PROJECTILE_FRAME_HEIGHT / PROJECTILE_FRAME_WIDTH;
inline constexpr float PLAYER_COLLIDER_SCALE = 0.8f;
inline constexpr float PROJECTILE_COLLIDER_SCALE = 0.85f;
inline constexpr float PROJECTILE_SPEED = 1100.0f;
inline constexpr double FIRE_RATE = 6.0;

};

#endif /* CONSTANTS_HPP_ */

