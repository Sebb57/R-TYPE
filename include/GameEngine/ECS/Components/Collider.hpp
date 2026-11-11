/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Collider.hpp
*/

#pragma once
#ifndef COLLIDER_HPP_
    #define COLLIDER_HPP_

    #include "plugins/AComponent.hpp"
    #include "../Utils.hpp"

namespace OrByte {

    enum class ColliderShape
    {
        Box,
        Circle,
    };
    
    
    struct Collider : public OrByte::AComponent<OrByte::ComponentType::Collider>
    {
        Collider() = default;
        Collider(const Vector2& size, ColliderShape shape = ColliderShape::Box) : size(size), colliderShape(shape) {}
    
        Vector2 size{0.0f, 0.0f};
        ColliderShape colliderShape{ColliderShape::Box};

        void serialize(OrByte::Serializer& serializer) const final
        {
            serializer.object(this->size);
            serializer.value4b(this->colliderShape);
        }

        void deserialize(OrByte::Deserializer& deserializer) final
        {
            deserializer.object(this->size);
            deserializer.value4b(this->colliderShape);
        }
    };

} // namespace OrByte

#endif /* COLLIDER_HPP_ */
