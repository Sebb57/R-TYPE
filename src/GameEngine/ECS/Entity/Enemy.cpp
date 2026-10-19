/*
** EPITECH PROJECT, 2026
** OrByte
** File description:
** Ennemies.cpp
*/

#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Entity/Enemy.hpp"
#include "GameEngine/ECS/Managers/ComponentManager.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include "GameEngine/ECS/World.hpp"
#include <random>

OrByte::Enemy::Enemy(World *world) : _randomLine(0.0)
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

void OrByte::Enemy::createServerComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->_id, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<OrByte::Health>(this->_id, 100, 100);
    world->addComponentToEntity<OrByte::RemoveHealth>(this->_id, 10);
    world->addComponentToEntity<OrByte::Velocity>(this->_id, Vector2 {-10.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->_id, Vector2 {2000.0f, this->_randomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Owner>(this->_id, Team::Enemy);
}

void OrByte::Enemy::createClientComponent(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->_id, Vector2 {2000.0f, this->_randomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<OrByte::Sprite>(this->_id, "", Vector2(100.0f, 100.0f));
}

void OrByte::Enemy::destructEnemy(World *world)
{
    constexpr float OFFSCREEN_X = -100.0f;
    std::vector<int> entities = world->getExistingEntities();
    for (int id : entities) {
        OrByte::Transform *transform = world->getComponent<OrByte::Transform>(id);
        OrByte::Health *health = world->getComponent<OrByte::Health>(id);
        if (transform == nullptr || health == nullptr)
            continue;
        if (transform->position.x < OFFSCREEN_X)
            world->destroyEntity(id);
    }
}
