/*
** EPITECH PROJECT, 2026
** RType
** File description:
** EntityManager.cpp
*/

#include "GameEngine/ECS/Managers/EntityManager.hpp"

EntityManager::EntityManager() : _nextEntityId(-1) {}

int EntityManager::createEntity()
{
    int entityId = this->_nextEntityId++;

    this->_entities[entityId] = true;
    return entityId;
}

int EntityManager::createEntity(int entityId)
{
    this->_entities[entityId] = true;
    if (entityId >= this->_nextEntityId)
        this->_nextEntityId = entityId + 1;
    return entityId;
}

void EntityManager::destroyEntity(int entityId)
{
    this->_entities.erase(entityId);
}

bool EntityManager::entityExists(int entityId) const
{
    return this->_entities.find(entityId) != this->_entities.end();
}

std::vector<int> EntityManager::getExistingEntities() const
{
    std::vector<int> res;

    res.reserve(this->_entities.size());
    for (const auto& [key, value] : this->_entities)
        res.push_back(key);
    return res;
}
