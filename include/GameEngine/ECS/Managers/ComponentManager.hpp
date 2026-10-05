#pragma once
#include "IComponent.hpp"
#include <map>
#include <memory>

class ComponentManager
{
public:
    ComponentManager() = default;
    ~ComponentManager() = default;

    template <typename T, typename ... Args>
    void addComponentToEntity(int entityId, Args&& ... args);

    //exemple d'utilisation : addComponentToEntity<Transform>(entityId, Vector2(0, 0), Vector2(1, 1), 0.0f); car Transform a un constructeur qui prend ces paramètres : KK Alexis 

    template <typename T>
    void removeComponentFromEntity(int entityId);

    template <typename T>
    T* getComponent(int entityId);

    void removeAllComponentsFromEntity(int entityId);


private:
    std::map<ComponentType, std::map<int, std::unique_ptr<IComponent>>> globalComponents;
};

