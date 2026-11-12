/**
 * @file GamePlugins.hpp
 * @brief plugins the game was compiled with (set by the engine's compile options)
 *
 * @author Seb
 * @date 2026-11-11
**/

#ifndef GAMEENGINE_CLIENT_GAMEPLUGINS_HPP
#define GAMEENGINE_CLIENT_GAMEPLUGINS_HPP

#include <optional>
#include <string>
#include <vector>

namespace client {

[[nodiscard]] const std::optional<std::vector<std::string>>& compiledSystems();
[[nodiscard]] const std::optional<std::vector<std::string>>& compiledComponents();

} // namespace client

#endif // GAMEENGINE_CLIENT_GAMEPLUGINS_HPP
