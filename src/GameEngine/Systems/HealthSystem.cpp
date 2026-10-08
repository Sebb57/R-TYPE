/*
** EPITECH PROJECT, 2026
** RType
** File description:
** HealthSystem.cpp
*/

#include "GameEngine/ECS/Components/RegenHealth.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/Systems/HealthSystem.hpp"
#include <vector>
#include <iostream>

OrByte::HealthSystem::HealthSystem(World& world) : _world(world)
{}

bool OrByte::HealthSystem::_handleDeath(Health* health, int entity)
{
    if (health->currentHealth <= 0) {
        this->_world.destroyEntity(entity);
        return true;
    }
    return false;
}

void OrByte::HealthSystem::_handleRegenHealth(Health* health, int entity)
{
    RegenHealth* regen = this->_world.getComponent<RegenHealth>(entity);
    if (regen == nullptr)
        return;

    health->currentHealth += regen->regenAmount;
    if (health->currentHealth > health->maxHealth)
        health->currentHealth = health->maxHealth;
    this->_world.removeComponentFromEntity<RegenHealth>(entity);
}

void OrByte::HealthSystem::_handleRemoveHealth(Health* health, int entity)
{
    RemoveHealth* rh = this->_world.getComponent<RemoveHealth>(entity);
    if (rh == nullptr)
        return;

    health->currentHealth -= rh->hitDamage;
    this->_world.removeComponentFromEntity<RemoveHealth>(entity);
}

void OrByte::HealthSystem::update()
{
    std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity: entities) {
        Health* health = this->_world.getComponent<Health>(entity);
        if (health == nullptr)
            continue;

        this->_handleRegenHealth(health, entity);
        this->_handleRemoveHealth(health, entity);
        if (this->_handleDeath(health, entity))
            continue;
    }
}
