#pragma once
#include "plugins/IComponent.hpp"
#include <map>
#include <memory>
#include <vector>

class ComponentManager
{
    std::map<OrByte::ComponentType, std::map<int, std::unique_ptr<OrByte::IComponent>>> _globalComponents;

    public:
        ComponentManager() = default;
        ~ComponentManager() = default;

        template <typename T, typename ... Args>
        void addComponentToEntity(int entityId, Args&& ... args) {
            OrByte::ComponentType type = T::Type;
            auto component = std::make_unique<T>(std::forward<Args>(args)...);

            this->_globalComponents[type][entityId] = std::move(component);
        }

        void addComponentToEntity(int entityId, std::unique_ptr<OrByte::IComponent> component);
        template <typename T>
        void removeComponentFromEntity(int entityId) {
            OrByte::ComponentType type = T::Type;

            auto it = this->_globalComponents.find(type);
            if (it != this->_globalComponents.end()) {
                it->second.erase(entityId);
            }
        }

        template <typename T>
        T* getComponent(int entityId) {
            OrByte::ComponentType type = T::Type;

            auto it = this->_globalComponents.find(type);
            if (it != this->_globalComponents.end()) {
                auto entityIt = it->second.find(entityId);
                if (entityIt != it->second.end()) {
                    return static_cast<T*>(entityIt->second.get());
                }
            }

            return nullptr;
        }
        

        template <typename T>
        bool setComponent(int entityId, T component) {
            T* existing = getComponent<T>(entityId);
            if (!existing)
                return false;
            *existing = std::move(component);
            return true;
        }

        [[nodiscard]] std::vector<OrByte::ComponentType> getComponentTypesOfEntity(int entityId) const
        {
            std::vector<OrByte::ComponentType> types;
            for (const auto& [type, entities] : _globalComponents)
                if (entities.count(entityId) != 0)
                    types.push_back(type);
            return types;
        }

        [[nodiscard]] OrByte::IComponent* getComponent(int entityId, OrByte::ComponentType type) const;
        [[nodiscard]] std::vector<OrByte::IComponent*> getComponents(int entityId) const;
        [[nodiscard]] std::vector<int> getEntitiesWith(OrByte::ComponentType type) const;

        void removeAllComponentsFromEntity(int entityId);
};
