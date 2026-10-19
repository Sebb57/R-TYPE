/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Player.cpp
*/

#include "GameEngine/ECS/Entity/Player.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Utils.hpp"

OrByte::Player::Player(World *world)
{
    this->_id = world->createEntity();
}

void OrByte::Player::createServerComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->_id, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Health>(this->_id, 100, 100);
    world->addComponentToEntity<Velocity>(this->_id, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Transform>(this->_id, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Owner>(this->_id, Team::Player);
}

void OrByte::Player::createClientComponent(World *world)
{
    world->addComponentToEntity<Transform>(this->_id, Vector2{0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Sprite>(this->_id, "", Vector2(100.0f, 100.0f));
}
