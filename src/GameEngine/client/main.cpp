/**
 * @file main.cpp
 * @brief client entry point
 *
 * @author Seb
 * @date 2026-10-13
**/

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include "GameEngine/ECS/ComponentPlugins.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/WorldSerializer.hpp"
#include "GameEngine/client/Client.hpp"
#include "constants.hpp"

namespace {

server::GameInstance::Setup engineGame(std::filesystem::path path, const OrByte::ComponentFactories& factories)
{
    return [path = std::move(path), &factories](World& world) {
        OrByte::deserializeWorld(world, path, factories);
    };
}

} // namespace

int main()
{
    try {
        if (!std::filesystem::is_regular_file(OrByte::constants::GAME_WORLD))
            throw std::runtime_error("error reading world file");
        const OrByte::ComponentPlugins components = OrByte::loadComponentPlugins(OrByte::constants::COMPONENTS_DIR);
        client::ClientConfig config;
        client::Client client(std::move(config), engineGame(OrByte::constants::GAME_WORLD, components.factories));
        client.run();
        return OrByte::constants::SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr <<  e.what() << std::endl;
        return OrByte::constants::FAILURE;
    }
}
