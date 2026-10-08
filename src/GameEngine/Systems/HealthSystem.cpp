/*
** EPITECH PROJECT, 2026
** RType
** File description:
** HealthSystem.cpp
*/

#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "GameEngine/ECS/Components/Poison.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Wither.hpp"
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

void OrByte::HealthSystem::_handlePoison(Health* health, int entity)
{
    Poison* poison = this->_world.getComponent<Poison>(entity);
    if (poison == nullptr)
        return;

    if (this->_poisonCooldowns.contains(entity)) {
        double cooldown = this->_poisonCooldowns[entity].getElapsedTime();

        if (cooldown >= poison->cooldown) {
            health->currentHealth -= poison->damage;
            if (health->currentHealth < 1)
                health->currentHealth = 1;
            poison->duration -= poison->cooldown;

            if (poison->cooldown > poison->duration) {
                this->_poisonCooldowns.erase(entity);
                this->_world.removeComponentFromEntity<Poison>(entity);
                return;
            }

            this->_poisonCooldowns[entity].reset();
        }
    } else {
        this->_poisonCooldowns[entity] = Chronometer();
        this->_poisonCooldowns[entity].reset();
    }
}

void OrByte::HealthSystem::_handleWither(Health* health, int entity)
{
    Wither* wither = this->_world.getComponent<Wither>(entity);
    if (wither == nullptr)
        return;

    if (this->_witherCooldowns.contains(entity)) {
        double cooldown = this->_witherCooldowns[entity].getElapsedTime();

        if (cooldown >= wither->cooldown) {
            health->currentHealth -= wither->damage;
            wither->duration -= wither->cooldown;

            if (wither->cooldown > wither->duration) {
                this->_witherCooldowns.erase(entity);
                this->_world.removeComponentFromEntity<Wither>(entity);
                return;
            }

            this->_witherCooldowns[entity].reset();
        }
    } else {
        this->_witherCooldowns[entity] = Chronometer();
        this->_witherCooldowns[entity].reset();
    }
}

void OrByte::HealthSystem::_handleAddHealth(Health* health, int entity)
{
    AddHealth* ah = this->_world.getComponent<AddHealth>(entity);
    if (ah == nullptr)
        return;

    health->currentHealth += ah->regenAmount;
    if (health->currentHealth > health->maxHealth)
        health->currentHealth = health->maxHealth;
    this->_world.removeComponentFromEntity<AddHealth>(entity);
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

        // Handle damage
        this->_handleRemoveHealth(health, entity);
        this->_handlePoison(health, entity);
        this->_handleWither(health, entity);

        // Handle regen
        this->_handleAddHealth(health, entity);

        // Handle death
        if (this->_handleDeath(health, entity))
            continue;
    }
}
