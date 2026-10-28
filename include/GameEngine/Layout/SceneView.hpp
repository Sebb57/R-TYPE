/*
** EPITECH PROJECT, 2026
** RType
** File description:
** SceneView.hpp
*/

#pragma once
#ifndef SCENEVIEW_HPP_
    #define SCENEVIEW_HPP_

#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/Layout/Widget.hpp"
#include "plugins/IGraphic.hpp"
#include <functional>

namespace OrByte {

class SceneView : public OrByte::Widget {
public:
    using RenderFn = std::function<void(const OrByte::IGraphic&)>;
    using EventFn  = std::function<bool(const OrByte::Event&)>;

    SceneView(RenderFn render, EventFn onGameEvent = nullptr)
        : Widget("Scene"), _render(std::move(render)), _onGameEvent(std::move(onGameEvent)) {}

protected:
    void draw(const OrByte::IGraphic& graphicLib, Vector2u size) override;
    bool onEvent(const OrByte::Event& event, Vector2 local) override;

private:
    RenderFn _render;
    EventFn  _onGameEvent;
};

} // namespace OrByte

#endif /* SCENEVIEW_HPP_ */
