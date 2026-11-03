/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ComponentManager.cpp
*/

#include "GameEngine/ECS/Managers/ComponentManager.hpp"

void ComponentManager::addComponentToEntity(int entityId, std::unique_ptr<OrByte::IComponent> component)
{
    if (component == nullptr)
        return;
    OrByte::ComponentType type = component->getType();
    this->_globalComponents[type][entityId] = std::move(component);
}

OrByte::IComponent* ComponentManager::getComponent(int entityId, OrByte::ComponentType type) const
{
    auto it = this->_globalComponents.find(type);
    if (it == this->_globalComponents.end())
        return nullptr;
    auto entityIt = it->second.find(entityId);
    return entityIt != it->second.end() ? entityIt->second.get() : nullptr;
}

std::vector<OrByte::IComponent*> ComponentManager::getComponents(int entityId) const
{
    std::vector<OrByte::IComponent*> components;

    for (const auto& [type, entities] : this->_globalComponents) {
        auto it = entities.find(entityId);
        if (it != entities.end())
            components.push_back(it->second.get());
    }
    return components;
}

std::vector<int> ComponentManager::getEntitiesWith(OrByte::ComponentType type) const
{
    std::vector<int> entities;

    auto it = this->_globalComponents.find(type);
    if (it == this->_globalComponents.end())
        return entities;
    entities.reserve(it->second.size());
    for (const auto& [entityId, component] : it->second)
        entities.push_back(entityId);
    return entities;
}

void ComponentManager::removeComponentFromEntity(int entityId, OrByte::ComponentType type)
{
    auto it = this->_globalComponents.find(type);
    if (it != this->_globalComponents.end())
        it->second.erase(entityId);
}

void ComponentManager::removeAllComponentsFromEntity(int entityId)
{
    for (auto& [type, components] : this->_globalComponents) {
        components.erase(entityId);
    }
}
