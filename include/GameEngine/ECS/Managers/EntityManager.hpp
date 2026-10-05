#pragma once
#include "GameEngine/ECS/Components/IComponent.hpp"
#include <map>
#include <memory>

class EntityManager
{
public:
    EntityManager();
    ~EntityManager() = default;

    int createEntity();
    void destroyEntity(int entityId);
    bool entityExists(int entityId) const;

private:
    int nextEntityId;
    std::map<int, bool> entities;
};