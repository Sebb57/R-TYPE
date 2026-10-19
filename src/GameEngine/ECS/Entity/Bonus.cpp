/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Bonus.cpp
*/

#include "GameEngine/ECS/Entity/Bonus.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Managers/ComponentManager.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include "GameEngine/ECS/World.hpp"
#include <random>

OrByte::Bonus::Bonus(World *world)
{
    this->_id = world->createEntity();
    constexpr int LINE_LENGTH = 1980;
    constexpr int SLOTS = 4;
    constexpr int SLOT_SIZE = LINE_LENGTH / SLOTS;

    static std::linear_congruential_engine<std::uint_fast32_t, 16807, 0, 2147483647>
        engine{std::random_device{}()};
    std::uniform_int_distribution<int> slot(0, SLOTS - 1);
    this->_randomLine = slot(engine) * SLOT_SIZE;
}

void OrByte::Bonus::createServerBonus(World *world)
{
    world->addComponentToEntity<Collider>(this->_id, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<OrByte::Velocity>(this->_id, Vector2 {-5.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->_id, Vector2 {2000.0f, this->_randomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
}

void OrByte::Bonus::createClientBonus(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->_id, Vector2 {2000.0f, this->_randomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<OrByte::Sprite>(this->_id, "", Vector2(100.0f, 100.0f));
}
