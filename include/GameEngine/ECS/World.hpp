#pragma once
#include "GameEngine/ECS/Components/IComponent.hpp"
#include "Managers/EntityManager.hpp"
#include "Managers/ComponentManager.hpp"

class World
{
private:
    EntityManager entityManager;
    ComponentManager componentManager;
    
public:
    World() = default;
    ~World() = default;

    int createEntity();
    void destroyEntity(int entityId);
    bool entityExists(int entityId) const;

    template <typename T, typename ... Args>
    void addComponentToEntity(int entityId, Args&& ... args);
    template <typename T>
    T* getComponent(int entityId);
    template <typename T>
    void removeComponentFromEntity(int entityId);
};