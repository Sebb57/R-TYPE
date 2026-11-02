/*
** EPITECH PROJECT, 2026
** RType
** File description:
** SceneView.cpp
*/

#include "GameEngine/Layout/SceneView.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cmath>

namespace {

float gridSpacing(float zoom)
{
    const float minimumWorldSpacing = 40.0F / zoom;
    const float power = std::pow(10.0F, std::floor(std::log10(minimumWorldSpacing)));
    for (const float multiplier : {1.0F, 2.0F, 5.0F, 10.0F}) {
        const float spacing = power * multiplier;
        if (spacing >= minimumWorldSpacing)
            return spacing;
    }
    return power * 10.0F;
}

} // namespace

OrByte::SceneView::SceneView(World& world)
    : Panel("Scene", 0), _world(world),
      _viewCenter(static_cast<float>(constants::SERVER_WINDOW_X) / 2.0F,
          static_cast<float>(constants::SERVER_WINDOW_Y) / 2.0F)
{
    this->setResizable(false);
}

Vector2 OrByte::SceneView::_screenPosition(const Transform& transform, Vector2u size) const
{
    const float areaHeight = std::max(0.0F, static_cast<float>(size.y) - HEADER_HEIGHT);
    return {
        static_cast<float>(size.x) / 2.0F + (transform.position.x - this->_viewCenter.x) * this->_zoom,
        HEADER_HEIGHT + areaHeight / 2.0F + (transform.position.y - this->_viewCenter.y) * this->_zoom,
    };
}

void OrByte::SceneView::_drawEntity(const IGraphic& graphic, int entity, Vector2u size) const
{
    const Transform *worldTransform = this->_world.getComponent<Transform>(entity);
    if (worldTransform == nullptr)
        return;

    const Vector2 position = this->_screenPosition(*worldTransform, size);
    const Transform viewTransform(position,
        {worldTransform->scale.x * this->_zoom, worldTransform->scale.y * this->_zoom},
        worldTransform->rotation);

    if (const Animator *animator = this->_world.getComponent<Animator>(entity)) {
        const Animation *animation = animator->getCurrentAnimation();
        if (animation == nullptr || animation->frameCount == 0)
            return;

        if (animation->isSpriteSheet) {
            if (animation->columns == 0 || animation->frameSize.x <= 0.0F || animation->frameSize.y <= 0.0F)
                return;
            const std::size_t frame = animation->firstFrame;
            const Rect frameRect(
                static_cast<int>((frame % animation->columns) * animation->frameSize.x),
                static_cast<int>((frame / animation->columns) * animation->frameSize.y),
                static_cast<int>(animation->frameSize.x),
                static_cast<int>(animation->frameSize.y));
            const Vector2 displaySize = animation->displaySize.x > 0.0F && animation->displaySize.y > 0.0F
                ? animation->displaySize : animation->frameSize;
            graphic.drawImage(viewTransform,
                {displaySize.x * std::abs(worldTransform->scale.x) * this->_zoom,
                    displaySize.y * std::abs(worldTransform->scale.y) * this->_zoom},
                animation->spriteSheetPath, frameRect);
            return;
        }

        if (animation->spritePaths.empty() || animation->frameSize.x <= 0.0F || animation->frameSize.y <= 0.0F)
            return;
        graphic.drawImage(viewTransform,
            {animation->frameSize.x * std::abs(worldTransform->scale.x) * this->_zoom,
                animation->frameSize.y * std::abs(worldTransform->scale.y) * this->_zoom},
            animation->spritePaths.front());
        return;
    }

    const Sprite *sprite = this->_world.getComponent<Sprite>(entity);
    if (sprite == nullptr || sprite->texturePath.empty() || sprite->size.x <= 0.0F || sprite->size.y <= 0.0F)
        return;
    graphic.drawImage(viewTransform,
        {sprite->size.x * std::abs(worldTransform->scale.x) * this->_zoom,
            sprite->size.y * std::abs(worldTransform->scale.y) * this->_zoom},
        sprite->texturePath);
}

void OrByte::SceneView::draw(const OrByte::IGraphic& graphicLib, Vector2u size)
{
    Panel::draw(graphicLib, size);
    if (size.y <= HEADER_HEIGHT)
        return;

    const float width = static_cast<float>(size.x);
    const float areaHeight = static_cast<float>(size.y) - HEADER_HEIGHT;
    const float centerX = width / 2.0F;
    const float centerY = HEADER_HEIGHT + areaHeight / 2.0F;
    const float spacing = gridSpacing(this->_zoom);
    const float left = this->_viewCenter.x - centerX / this->_zoom;
    const float right = this->_viewCenter.x + centerX / this->_zoom;
    const float top = this->_viewCenter.y - (centerY - HEADER_HEIGHT) / this->_zoom;
    const float bottom = this->_viewCenter.y + (static_cast<float>(size.y) - centerY) / this->_zoom;
    const auto firstX = static_cast<long long>(std::ceil(left / spacing));
    const auto lastX = static_cast<long long>(std::floor(right / spacing));
    const auto firstY = static_cast<long long>(std::ceil(top / spacing));
    const auto lastY = static_cast<long long>(std::floor(bottom / spacing));

    for (long long index = firstX; index <= lastX; ++index) {
        const float worldX = static_cast<float>(index) * spacing;
        const float x = centerX + (worldX - this->_viewCenter.x) * this->_zoom;
        const bool axis = std::abs(worldX) < 0.01F;
        const bool major = index % 5 == 0;
        const RGBA color = axis ? RGBA{105, 125, 155, 255}
            : major ? RGBA{67, 78, 96, 255} : RGBA{49, 58, 72, 255};
        graphicLib.drawRectangle(Transform({x, centerY}, {1.0F, 1.0F}, 0.0F), {1.0F, areaHeight}, color);
    }
    for (long long index = firstY; index <= lastY; ++index) {
        const float worldY = static_cast<float>(index) * spacing;
        const float y = centerY + (worldY - this->_viewCenter.y) * this->_zoom;
        const bool axis = std::abs(worldY) < 0.01F;
        const bool major = index % 5 == 0;
        const RGBA color = axis ? RGBA{105, 125, 155, 255}
            : major ? RGBA{67, 78, 96, 255} : RGBA{49, 58, 72, 255};
        graphicLib.drawRectangle(Transform({centerX, y}, {1.0F, 1.0F}, 0.0F), {width, 1.0F}, color);
    }

    for (const int entity : this->_world.getExistingEntities())
        this->_drawEntity(graphicLib, entity, size);
}

bool OrByte::SceneView::onEvent(const OrByte::Event& event, Vector2 local)
{
    if (this->_isPanning) {
        if (event.type == Event::Type::MouseMoved) {
            const float deltaX = local.x - this->_lastMousePosition.x;
            const float deltaY = local.y - this->_lastMousePosition.y;
            this->_viewCenter.x -= deltaX / this->_zoom;
            this->_viewCenter.y -= deltaY / this->_zoom;
            this->_lastMousePosition = local;
            return true;
        }
        if (event.type == Event::Type::MouseButtonReleased && event.mouseButton == Event::MouseButton::Middle) {
            this->_isPanning = false;
            return true;
        }
    }

    if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Middle
        && local.y >= HEADER_HEIGHT) {
        this->_isPanning = true;
        this->_lastMousePosition = local;
        return true;
    }

    if (event.type != Event::Type::MouseWheelScrolled || event.wheelDelta == 0.0)
        return false;

    const float areaHeight = std::max(0.0F, static_cast<float>(this->getContentRect().h) - HEADER_HEIGHT);
    if (local.y < HEADER_HEIGHT || areaHeight <= 0.0F)
        return false;

    const float centerX = static_cast<float>(this->getContentRect().w) / 2.0F;
    const float centerY = HEADER_HEIGHT + areaHeight / 2.0F;
    const Vector2 worldUnderMouse{
        this->_viewCenter.x + (local.x - centerX) / this->_zoom,
        this->_viewCenter.y + (local.y - centerY) / this->_zoom,
    };

    const float factor = event.wheelDelta > 0.0 ? ZOOM_FACTOR : 1.0F / ZOOM_FACTOR;
    this->_zoom = std::clamp(this->_zoom * factor, MIN_ZOOM, MAX_ZOOM);
    this->_viewCenter = {
        worldUnderMouse.x - (local.x - centerX) / this->_zoom,
        worldUnderMouse.y - (local.y - centerY) / this->_zoom,
    };
    return true;
}
