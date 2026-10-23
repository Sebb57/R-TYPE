/**
 * @file main.cpp
 * @brief client entry point
 *
 * @author Seb
 * @date 2026-10-13
**/

#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/client/Client.hpp"
#include "constants.hpp"

namespace {

server::GameInstance::Setup demoGame(std::string image)
{
    return [image = std::move(image)](World& world) {
        const int entity = world.createEntity();
        const Vector2 center(OrByte::constants::SERVER_WINDOW_X / 2.0F, OrByte::constants::SERVER_WINDOW_Y / 2.0F);
        world.addComponentToEntity<OrByte::Transform>(entity, center, Vector2(1.0F, 1.0F), 0.0F);
        world.addComponentToEntity<OrByte::Controllable>(entity, Vector2(96.0F, 96.0F), image);
    };
}

} // namespace

int main(int argc, char **argv)
{
    try {
        client::ClientConfig config;
        if (argc > 1)
            config.host = argv[1];
        if (argc > 2)
            config.username = argv[2];
        client::Client client(std::move(config), demoGame(""));
        client.run();
        return OrByte::constants::SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr <<  e.what() << std::endl;
        return OrByte::constants::FAILURE;
    }
}
