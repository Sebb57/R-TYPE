/**
 * @file GameWorld.hpp
 * @brief world the game was compiled with, embedded in the binary by cmake/GameEngine/EmbedWorld.cmake
 *
 * @author Seb
 * @date 2026-11-11
**/

#ifndef GAMEENGINE_CLIENT_GAMEWORLD_HPP
#define GAMEENGINE_CLIENT_GAMEWORLD_HPP

#include <string_view>

namespace client {

[[nodiscard]] std::string_view gameWorld();

} // namespace client

#endif // GAMEENGINE_CLIENT_GAMEWORLD_HPP
