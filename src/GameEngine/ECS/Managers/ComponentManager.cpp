/*
** EPITECH PROJECT, 2026
** RType
** File description:
** ComponentManager.cpp
*/

#include "GameEngine/ECS/Managers/ComponentManager.hpp"

void ComponentManager::removeAllComponentsFromEntity(int entityId) {
    for (auto& [type, components] : this->_globalComponents) {
        components.erase(entityId);
    }
}
