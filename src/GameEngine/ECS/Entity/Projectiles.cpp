/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Projectiles.cpp
*/

#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Entity/Projectiles.hpp"
#include "GameEngine/ECS/Utils.hpp"
#include <cmath>

OrByte::Projectiles::Projectiles(World *world, int shooterId, Team owner) : _owner(owner), _spawnPos({0.0f, 0.0f}), _velocity({0.0f, 0.0f})
{
    constexpr float SPAWN_OFFSET = 50.0f;
    constexpr float PROJECTILE_SPEED = 20.0f;
    this->_id = world->createEntity();
    const float dir = (owner == Team::Player) ? 1.0f : -1.0f;
    OrByte::Transform *shooterTransform = world->getComponent<OrByte::Transform>(shooterId);
    OrByte::Velocity *shooterVelocity = world->getComponent<OrByte::Velocity>(shooterId);

    if (shooterTransform != nullptr) {
        this->_spawnPos.x = shooterTransform->position.x + dir * SPAWN_OFFSET;
        this->_spawnPos.y = shooterTransform->position.y;
    }

    float shooterSpeed = 0.0f;
    if (shooterVelocity != nullptr)
        shooterSpeed = std::abs(shooterVelocity->velocity.x);
    this->_velocity.x = dir * (PROJECTILE_SPEED + shooterSpeed);
    this->_velocity.y = 0.0f;
}

void OrByte::Projectiles::createServerComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->_id, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<OrByte::Velocity>(this->_id, this->_velocity);
    world->addComponentToEntity<OrByte::Transform>(this->_id, this->_spawnPos, Vector2 {1.0f, 1.0f}, 0.0f);
    world->addComponentToEntity<OrByte::RemoveHealth>(this->_id, 10);
    world->addComponentToEntity<Owner>(this->_id, _owner);
}

void OrByte::Projectiles::createClientComponent(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->_id, this->_spawnPos, Vector2 {1.0f, 1.0f}, 0.0f);
    world->addComponentToEntity<OrByte::Sprite>(this->_id, "", Vector2(100.0f, 100.0f));
}

static bool applyDamage(World *world, int attacker, int victim)
{
    OrByte::RemoveHealth *damage = world->getComponent<OrByte::RemoveHealth>(attacker);
    OrByte::Health       *health = world->getComponent<OrByte::Health>(victim);

    if (damage == nullptr || health == nullptr)
        return false;

    health->currentHealth -= damage->hitDamage;
    return health->currentHealth <= 0;
}

void OrByte::Projectiles::checkCollision(World *world, Vector2 entity)
{
    const int idA = static_cast<int>(entity.x);
    const int idB = static_cast<int>(entity.y);

    Owner *ownerA = world->getComponent<Owner>(idA);
    Owner *ownerB = world->getComponent<Owner>(idB);
    if (ownerA && ownerB && ownerA->team == ownerB->team)
        return;

    bool deadA = applyDamage(world, idB, idA);
    bool deadB = applyDamage(world, idA, idB);
}
