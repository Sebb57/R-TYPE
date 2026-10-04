#include "ComponentManager.hpp"

template <typename T, typename ... Args>
void ComponentManager::addComponentToEntity(int entityId, Args&& ... args) {
    ComponentType type = T::Type;
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    globalComponents[type][entityId] = std::move(component);
}

template <typename T>
void ComponentManager::removeComponentFromEntity(int entityId) {
    ComponentType type = T::Type;
    auto it = globalComponents.find(type);
    if (it != globalComponents.end()) {
        it->second.erase(entityId);
    }
}

template <typename T>
T* ComponentManager::getComponent(int entityId) {
    ComponentType type = T::Type;
    auto it = globalComponents.find(type);
    if (it != globalComponents.end()) {
        auto entityIt = it->second.find(entityId);
        if (entityIt != it->second.end()) {
            return static_cast<T*>(entityIt->second.get());
        }
    }
    return nullptr;
}

void ComponentManager::removeAllComponentsFromEntity(int entityId) {
    for (auto& [type, components] : globalComponents) {
        components.erase(entityId);
    }
}
