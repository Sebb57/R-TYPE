/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Detail.cpp
*/

#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Orbit.hpp"
#include "GameEngine/ECS/Components/Poison.hpp"
#include "GameEngine/ECS/Components/RegenHealth.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Sinus.hpp"
#include "GameEngine/ECS/Components/Spiral.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Wither.hpp"
#include "GameEngine/Detail/Detail.hpp"
#include "GameEngine/ECS/World.hpp"
#include "GameEngine/ECS/Managers/ComponentManager.hpp"

OrByte::Detail::Detail(World *world, int id)
{
    for (ComponentType type : world->getComponentTypesOfEntity(id)) {
        this->_componentUse.push_back(type);
    }
}

void OrByte::Detail::affichageDetail(World *world, int id)
{
    for (int i = 0; i < this->_componentUse.size(); i++) {
        switch (this->_componentUse[i]) {
            case ComponentType::AddHealth: {
                auto* t = world->getComponent<AddHealth*>(id);
                break;
            }
            case ComponentType::Animator: {
                auto* t = world->getComponent<Animator*>(id);
                break;
            }
            case ComponentType::Collider: {
                auto* t = world->getComponent<Collider*>(id);
                break;
            }
            case ComponentType::Health: {
                auto* t = world->getComponent<Health*>(id);
                break;
            }
            case ComponentType::Orbit: {
                auto* t = world->getComponent<Orbit*>(id);
                break;
            }
            case ComponentType::Poison: {
                auto* t = world->getComponent<Poison*>(id);
                break;
            }
            case ComponentType::RegenHealth: {
                auto* t = world->getComponent<RegenHealth*>(id);
                break;
            }
            case ComponentType::RemoveHealth: {
                auto* t = world->getComponent<RemoveHealth*>(id);
                break;
            }
            case ComponentType::Sinus: {
                auto* t = world->getComponent<Sinus*>(id);
                break;
            }
            case ComponentType::Spiral: {
                auto* t = world->getComponent<Spiral*>(id);
                break;
            }
            case ComponentType::Transform: {
                auto* t = world->getComponent<Transform*>(id);
                break;
            }
            case ComponentType::Velocity: {
                auto* t = world->getComponent<Velocity*>(id);
                break;
            }
            case ComponentType::Wither: {
                auto* t = world->getComponent<Wither*>(id);
                break;
            }
        }
    }
}
