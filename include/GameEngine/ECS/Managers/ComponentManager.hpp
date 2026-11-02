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

        //exemple d'utilisation : addComponentToEntity<Transform>(entityId, Vector2(0, 0), Vector2(1, 1), 0.0f); car Transform a un constructeur qui prend ces paramètres : KK Alexis 

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

        [[nodiscard]] std::vector<OrByte::IComponent*> getComponents(int entityId) const;
        [[nodiscard]] std::vector<int> getEntitiesWith(OrByte::ComponentType type) const;

        void removeAllComponentsFromEntity(int entityId);
};
