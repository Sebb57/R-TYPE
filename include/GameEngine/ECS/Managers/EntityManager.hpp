/*
** EPITECH PROJECT, 2026
** RType
** File description:
** EntityManager.hpp
*/

#pragma once
#ifndef ENTITYMANAGER_HPP_
    #define ENTITYMANAGER_HPP_

    #include <map>
    #include <vector>

class EntityManager {
    int _nextEntityId;
    std::map<int, bool> _entities;
        
    public:
        EntityManager();
        ~EntityManager() = default;

        int createEntity();
        void destroyEntity(int entityId);
        [[nodiscard]] bool entityExists(int entityId) const;
        [[nodiscard]] std::vector<int> getExistingEntities() const;
};

#endif /* ENTITYMANAGER_HPP_ */
