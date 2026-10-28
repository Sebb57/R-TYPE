/*
** EPITECH PROJECT, 2026
** RType
** File description:
** World.hpp
*/

#pragma once
#ifndef WORLD_HPP_
    #define WORLD_HPP_

    #include "plugins/IComponent.hpp"
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
    [[nodiscard]] bool entityExists(int entityId) const;
    [[nodiscard]] std::vector<int> getExistingEntities() const;

    template <typename T, typename ... Args>
    void addComponentToEntity(int entityId, Args&& ... args) {
        componentManager.addComponentToEntity<T>(entityId, std::forward<Args>(args)...);
    }
    void addComponentToEntity(int entityId, std::unique_ptr<OrByte::IComponent> component) {
        componentManager.addComponentToEntity(entityId, std::move(component));
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
