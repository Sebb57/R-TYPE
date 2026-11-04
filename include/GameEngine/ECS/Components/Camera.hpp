/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Camera.hpp
*/

#ifndef CAMERA_HPP_
    #define CAMERA_HPP_

    #include "plugins/AComponent.hpp"
    #include "../Utils.hpp"

namespace OrByte {

struct Camera : public OrByte::AComponent<OrByte::ComponentType::Camera>
{

    Camera() = default;
    Camera(float boundX, float boundY) : boundX(boundX), boundY(boundY) {}

    float boundX{0.0F};
    float boundY{0.0F};

    [[nodiscard]] Vector2 origin(const Vector2& target, const Vector2& view) const
    {
        return {clampToBound(target.x, view.x / 2.0F, this->boundX) - (view.x / 2.0F), clampToBound(target.y, view.y / 2.0F, this->boundY) - (view.y / 2.0F)};
    }

    void serialize(Serializer& serializer) const final
    {
        serializer.value4b(this->boundX);
        serializer.value4b(this->boundY);
    }

    void deserialize(Deserializer& deserializer) final
    {
        deserializer.value4b(this->boundX);
        deserializer.value4b(this->boundY);
    }
};

} // namespace OrByte

#endif /* CAMERA_HPP_ */
