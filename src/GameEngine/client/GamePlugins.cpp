/**
 * @file GamePlugins.cpp
 * @brief plugins the game was compiled with implementation
 *
 * @author Seb
 * @date 2026-11-11
**/

#include "GameEngine/client/GamePlugins.hpp"
#include <ranges>
#include <string_view>

namespace {

[[nodiscard]] std::vector<std::string> split(std::string_view list)
{
    std::vector<std::string> names;

    for (const auto part : std::views::split(list, ','))
        if (!part.empty())
            names.emplace_back(part.begin(), part.end());
    return names;
}

} // namespace

const std::optional<std::vector<std::string>>& client::compiledSystems()
{
#ifdef ORBYTE_GAME_SYSTEMS
    static const std::optional<std::vector<std::string>> systems = split(ORBYTE_GAME_SYSTEMS);
#else
    static const std::optional<std::vector<std::string>> systems;
#endif
    return systems;
}

const std::optional<std::vector<std::string>>& client::compiledComponents()
{
#ifdef ORBYTE_GAME_COMPONENTS
    static const std::optional<std::vector<std::string>> components = split(ORBYTE_GAME_COMPONENTS);
#else
    static const std::optional<std::vector<std::string>> components;
#endif
    return components;
}
