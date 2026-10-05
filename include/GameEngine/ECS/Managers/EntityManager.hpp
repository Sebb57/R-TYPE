/*
** EPITECH PROJECT, 2026
** RType
** File description:
** EntityManager.hpp
*/

#pragma once
#ifndef ENTITYMANAGER_HPP_
    #define ENTITYMANAGER_HPP_

    #include "GameEngine/ECS/Components/IComponent.hpp"
    #include <map>
    #include <memory>
    #include <vector>

class EntityManager {
    int _nextEntityId;
    std::map<int, bool> _entities;
        
    public:
        EntityManager();
        ~EntityManager() = default;

        int createEntity();
        void destroyEntity(int entityId);
        bool entityExists(int entityId) const;
        std::vector<int> getExistingEntities() const;
};

#endif /* ENTITYMANAGER_HPP_ */
