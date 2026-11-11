/**
 * @file main.cpp
 * @brief client entry point
 *
 * @author Seb
 * @date 2026-10-13
**/

#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include "GameEngine/ECS/ComponentPlugins.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/WorldSerializer.hpp"
#include "GameEngine/client/Client.hpp"
#include "GameEngine/client/GameWorld.hpp"
#include "constants.hpp"

namespace {

server::GameInstance::Setup engineGame(std::string_view data, const OrByte::ComponentFactories& factories)
{
    return [data, &factories](World& world) {
        std::istringstream stream{std::string(data)};

        OrByte::deserializeWorld(world, stream, factories, "the game world");
    };
}

} // namespace

int main()
{
    try {
        if (client::gameWorld().empty())
            throw OrByte::WorldSerializerException("This game has no world, compile it from the engine");
        const OrByte::ComponentPlugins components = OrByte::loadComponentPlugins(OrByte::constants::COMPONENTS_DIR);
        client::ClientConfig config;
        client::Client client(std::move(config), engineGame(client::gameWorld(), components.factories));
        client.run();
        return OrByte::constants::SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr <<  e.what() << std::endl;
        return OrByte::constants::FAILURE;
    }
}
