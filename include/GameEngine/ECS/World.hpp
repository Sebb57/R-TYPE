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
    #include "GameEngine/ECS/Utils.hpp"
    #include <optional>

class World {
    EntityManager entityManager;
    ComponentManager componentManager;
    std::optional<int> _selectedEntityId;
    std::optional<int> _pendingEntityDrag;

    void _releaseCamera(int newHolder);

public:
    World() = default;
    ~World() = default;

    int createEntity();
    int createEntity(int entityId);
    void destroyEntity(int entityId);
    void clearWorld();
    [[nodiscard]] bool entityExists(int entityId) const;
    [[nodiscard]] std::vector<int> getExistingEntities() const;

    [[nodiscard]] std::vector<OrByte::ComponentType> getComponentTypesOfEntity(int entityId) const {
        return componentManager.getComponentTypesOfEntity(entityId);
    }

    template <typename T, typename ... Args>
    void addComponentToEntity(int entityId, Args&& ... args) {
        if (T::Type == OrByte::ComponentType::Camera)
            this->_releaseCamera(entityId);
        componentManager.addComponentToEntity<T>(entityId, std::forward<Args>(args)...);
    }
    void addComponentToEntity(int entityId, std::unique_ptr<OrByte::IComponent> component) {
        if (component != nullptr && component->getType() == OrByte::ComponentType::Camera)
            this->_releaseCamera(entityId);
        componentManager.addComponentToEntity(entityId, std::move(component));
    }
    template <typename T>
    T* getComponent(int entityId) {
        return componentManager.getComponent<T>(entityId);
    }
    [[nodiscard]] OrByte::IComponent* getComponent(int entityId, OrByte::ComponentType type) const {
        return componentManager.getComponent(entityId, type);
    }
    [[nodiscard]] std::vector<OrByte::IComponent*> getComponents(int entityId) const {
        return componentManager.getComponents(entityId);
    }
    [[nodiscard]] static bool isComponentRemovable(OrByte::ComponentType type) {
        return type != OrByte::ComponentType::Name
            && type != OrByte::ComponentType::Transform;
    }
    template <typename T>
    void removeComponentFromEntity(int entityId) {
        if (isComponentRemovable(T::Type))
            componentManager.removeComponentFromEntity<T>(entityId);
    }
    void removeComponentFromEntity(int entityId, OrByte::ComponentType type) {
        if (isComponentRemovable(type))
            componentManager.removeComponentFromEntity(entityId, type);
    }

    int ensureCamera();
    [[nodiscard]] std::optional<int> getCameraEntity() const;
    [[nodiscard]] Vector2 getCameraOrigin(const Vector2& view);

    void setEntitySelected(int entityId) {
        this->_selectedEntityId = entityId;
    }
    void clearSelectedEntity() {
        this->_selectedEntityId.reset();
    }
    [[nodiscard]] std::optional<int> getSelectedEntity() const {
        return this->_selectedEntityId;
    }
    void setPendingEntityDrag(int entityId) {
        this->_pendingEntityDrag = entityId;
    }
    void clearPendingEntityDrag() {
        this->_pendingEntityDrag.reset();
    }
    [[nodiscard]] std::optional<int> getPendingEntityDrag() const {
        return this->_pendingEntityDrag;
    }
};

#endif /* WORLD_HPP_ */
