/*
** EPITECH PROJECT, 2026
** RType
** File description:
** World.hpp
*/

#pragma once
#ifndef WORLD_HPP_
    #define WORLD_HPP_

    #include "GameEngine/ECS/Components/IComponent.hpp"
    #include "Managers/EntityManager.hpp"
    #include "Managers/ComponentManager.hpp"
    
class World {
    EntityManager entityManager;
    ComponentManager componentManager;
    
public:
    World() = default;
    ~World() = default;

    int createEntity();
    void destroyEntity(int entityId);
    bool entityExists(int entityId) const;
    std::vector<int> getExistingEntities() const;

    template <typename T, typename ... Args>
    void addComponentToEntity(int entityId, Args&& ... args) {
        componentManager.addComponentToEntity<T>(entityId, std::forward<Args>(args)...);
    }
    template <typename T>
    T* getComponent(int entityId) {
        return componentManager.getComponent<T>(entityId);
    }
    template <typename T>
    void removeComponentFromEntity(int entityId) {
        componentManager.removeComponentFromEntity<T>(entityId);
    }
};

#endif /* WORLD_HPP_ */
