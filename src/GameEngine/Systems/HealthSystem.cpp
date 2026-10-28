/*
** EPITECH PROJECT, 2026
** RType
** File description:
** HealthSystem.cpp
*/

#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "GameEngine/ECS/Components/Poison.hpp"
#include "GameEngine/ECS/Components/RegenHealth.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Wither.hpp"
#include "GameEngine/Systems/HealthSystem.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"
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

void OrByte::HealthSystem::_handleRemoveHealth(Health* health, int entity)
{
    RemoveHealth* rh = this->_world.getComponent<RemoveHealth>(entity);
    if (rh == nullptr)
        return;

    health->currentHealth -= rh->hitDamage;
    this->_world.removeComponentFromEntity<RemoveHealth>(entity);
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

void OrByte::HealthSystem::_handleRegenHealth(Health* health, int entity)
{
    RegenHealth* regen = this->_world.getComponent<RegenHealth>(entity);
    if (regen == nullptr)
        return;

    if (this->_regenCooldowns.contains(entity)) {
        double cooldown = this->_regenCooldowns[entity].getElapsedTime();

        if (cooldown >= regen->cooldown) {
            health->currentHealth += regen->regenAmount;
            if (health->currentHealth > health->maxHealth)
                health->currentHealth = health->maxHealth;
            regen->duration -= regen->cooldown;

            if (regen->cooldown > regen->duration) {
                this->_regenCooldowns.erase(entity);
                this->_world.removeComponentFromEntity<RegenHealth>(entity);
                return;
            }

            this->_regenCooldowns[entity].reset();
        }
    } else {
        this->_regenCooldowns[entity] = Chronometer();
        this->_regenCooldowns[entity].reset();
    }
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
        this->_handleRegenHealth(health, entity);

        // Handle death
        if (this->_handleDeath(health, entity))
            continue;
    }
}

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::SYSTEMS;
    }

    OrByte::ISystem *factory(World& world)
    {
        return new OrByte::HealthSystem(world);
    }
}
