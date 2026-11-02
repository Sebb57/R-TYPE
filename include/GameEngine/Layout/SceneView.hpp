/*
** EPITECH PROJECT, 2026
** RType
** File description:
** SceneView.hpp
*/

#pragma once
#ifndef SCENEVIEW_HPP_
    #define SCENEVIEW_HPP_

#include "GameEngine/ECS/World.hpp"
#include "GameEngine/Layout/Panel.hpp"
#include "plugins/IGraphic.hpp"

namespace OrByte {

class SceneView final : public Panel {
public:
    explicit SceneView(World& world);
    ~SceneView() override = default;

protected:
    void draw(const OrByte::IGraphic& graphicLib, Vector2u size) override;
    bool onEvent(const OrByte::Event& event, Vector2 local) override;
    [[nodiscard]] bool capturesMouse() const override { return this->_isPanning; }

private:
    static constexpr float HEADER_HEIGHT = 24.0F;
    static constexpr float MIN_ZOOM = 0.1F;
    static constexpr float MAX_ZOOM = 4.0F;
    static constexpr float ZOOM_FACTOR = 1.15F;

    World& _world;
    float _zoom = 0.5F;
    Vector2 _viewCenter;
    Vector2 _lastMousePosition;
    bool _isPanning = false;

    [[nodiscard]] Vector2 _screenPosition(const Transform& transform, Vector2u size) const;
    [[nodiscard]] Vector2 _entitySize(int entity, const Transform& transform) const;
    [[nodiscard]] bool _hitTestEntity(int entity, Vector2 worldPosition) const;
    void _drawEntity(const IGraphic& graphic, int entity, Vector2u size) const;
    void _drawSelection(const IGraphic& graphic, Vector2u size) const;
    void _drawCamera(const IGraphic& graphic, Vector2u size) const;
};

} // namespace OrByte

#endif /* SCENEVIEW_HPP_ */
