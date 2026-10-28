/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ComponentManager.cpp
*/

#include "GameEngine/ECS/Managers/ComponentManager.hpp"

void ComponentManager::addComponentToEntity(int entityId, std::unique_ptr<OrByte::IComponent> component) {
    if (component == nullptr)
        return;
    OrByte::ComponentType type = component->getType();

    this->_globalComponents[type][entityId] = std::move(component);
}

void ComponentManager::removeAllComponentsFromEntity(int entityId) {
    for (auto& [type, components] : this->_globalComponents) {
        components.erase(entityId);
    }
}
