/*
** EPITECH PROJECT, 2026
** RType
** File description:
** SceneView.cpp
*/

#include "GameEngine/Layout/SceneView.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/Components/Camera.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

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

Vector2 OrByte::SceneView::_worldPositionAt(Vector2 local) const
{
    const Rect& content = this->getContentRect();
    const float areaHeight = std::max(0.0F, static_cast<float>(content.h) - HEADER_HEIGHT);
    const float centerX = static_cast<float>(content.w) / 2.0F;
    const float centerY = HEADER_HEIGHT + areaHeight / 2.0F;
    return {
        this->_viewCenter.x + (local.x - centerX) / this->_zoom,
        this->_viewCenter.y + (local.y - centerY) / this->_zoom,
    };
}

Vector2 OrByte::SceneView::_entitySize(int entity, const Transform& transform) const
{
    Vector2 size;
    if (const Animator *animator = this->_world.getComponent<Animator>(entity)) {
        const Animation *animation = animator->getCurrentAnimation();
        if (animation == nullptr || animation->frameCount == 0)
            return {};
        if (animation->isSpriteSheet) {
            size = animation->displaySize.x > 0.0F && animation->displaySize.y > 0.0F
                ? animation->displaySize : animation->frameSize;
        } else {
            if (animation->spritePaths.empty())
                return {};
            size = animation->frameSize;
        }
    } else if (const Sprite *sprite = this->_world.getComponent<Sprite>(entity)) {
        if (sprite->texturePath.empty())
            return {};
        size = sprite->size;
    }
    return {size.x * std::abs(transform.scale.x), size.y * std::abs(transform.scale.y)};
}

bool OrByte::SceneView::_hitTestEntity(int entity, Vector2 worldPosition) const
{
    const Transform *transform = this->_world.getComponent<Transform>(entity);
    if (transform == nullptr)
        return false;

    const float deltaX = worldPosition.x - transform->position.x;
    const float deltaY = worldPosition.y - transform->position.y;
    if (this->_world.getComponent<Camera>(entity) != nullptr) {
        const float pickRadius = 12.0F / this->_zoom;
        return deltaX * deltaX + deltaY * deltaY <= pickRadius * pickRadius;
    }
    if (const Collider *collider = this->_world.getComponent<Collider>(entity)) {
        const float width = collider->size.x * std::abs(transform->scale.x);
        const float height = collider->size.y * std::abs(transform->scale.y);
        if (collider->colliderShape == ColliderShape::Circle) {
            const float radius = width / 2.0F;
            if (deltaX * deltaX + deltaY * deltaY <= radius * radius)
                return true;
        } else if (std::abs(deltaX) <= width / 2.0F && std::abs(deltaY) <= height / 2.0F) {
            return true;
        }
    }

    const Vector2 size = this->_entitySize(entity, *transform);
    if (size.x <= 0.0F || size.y <= 0.0F)
        return false;
    const float radians = transform->rotation * std::numbers::pi_v<float> / 180.0F;
    const float localX = deltaX * std::cos(radians) + deltaY * std::sin(radians);
    const float localY = -deltaX * std::sin(radians) + deltaY * std::cos(radians);
    return std::abs(localX) <= size.x / 2.0F && std::abs(localY) <= size.y / 2.0F;
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

void OrByte::SceneView::_drawCamera(const IGraphic& graphic, Vector2u size) const
{
    const std::optional<int> cameraEntity = this->_world.getCameraEntity();
    if (!cameraEntity)
        return;

    const Transform *transform = this->_world.getComponent<Transform>(*cameraEntity);
    const Camera *camera = this->_world.getComponent<Camera>(*cameraEntity);
    if (transform == nullptr || camera == nullptr)
        return;

    const Vector2 center = this->_screenPosition(*transform, size);
    const Transform viewTransform(center, {1.0F, 1.0F}, transform->rotation);
    const float viewWidth = static_cast<float>(constants::SERVER_WINDOW_X) * this->_zoom;
    const float viewHeight = static_cast<float>(constants::SERVER_WINDOW_Y) * this->_zoom;
    graphic.drawRectangle(viewTransform, {viewWidth, viewHeight}, RGBA{0, 0, 0, 0}, 2, RGBA{70, 205, 255, 255});

    constexpr float indicatorRadius = 5.0F;
    constexpr float indicatorArm = 16.0F;
    const RGBA indicatorColor{100, 225, 255, 255};
    graphic.drawCircle(viewTransform, indicatorRadius, indicatorColor);
    graphic.drawRectangle(Transform({center.x, center.y}, {1.0F, 1.0F}, 0.0F),
        {indicatorArm, 2.0F}, indicatorColor);
    graphic.drawRectangle(Transform({center.x, center.y}, {1.0F, 1.0F}, 0.0F),
        {2.0F, indicatorArm}, indicatorColor);
}

void OrByte::SceneView::_drawSelection(const IGraphic& graphic, Vector2u size) const
{
    const std::optional<int> selectedEntity = this->_world.getSelectedEntity();
    if (!selectedEntity || !this->_world.entityExists(*selectedEntity))
        return;
    const int entity = *selectedEntity;

    const Transform *transform = this->_world.getComponent<Transform>(entity);
    if (transform == nullptr)
        return;

    const Vector2 position = this->_screenPosition(*transform, size);
    const Transform viewTransform(position, {1.0F, 1.0F}, transform->rotation);
    const Transform hitboxTransform(position, {1.0F, 1.0F}, 0.0F);
    const Collider *collider = this->_world.getComponent<Collider>(entity);
    if (collider != nullptr) {
        const RGBA hitboxColor{80, 255, 100, 255};
        const float width = collider->size.x * std::abs(transform->scale.x) * this->_zoom;
        const float height = collider->size.y * std::abs(transform->scale.y) * this->_zoom;
        if (collider->colliderShape == ColliderShape::Circle) {
            graphic.drawCircle(hitboxTransform, width / 2.0F, RGBA{0, 0, 0, 0}, 2, hitboxColor);
        } else {
            graphic.drawRectangle(hitboxTransform, {width, height}, RGBA{0, 0, 0, 0}, 2, hitboxColor);
        }
    }

    const Vector2 entitySize = this->_entitySize(entity, *transform);
    const RGBA selectionColor{255, 220, 45, 255};
    if (entitySize.x > 0.0F && entitySize.y > 0.0F) {
        graphic.drawRectangle(viewTransform,
            {entitySize.x * this->_zoom, entitySize.y * this->_zoom},
            RGBA{0, 0, 0, 0}, 2, selectionColor);
        return;
    }

    if (this->_world.getComponent<Camera>(entity) != nullptr) {
        graphic.drawRectangle(viewTransform,
            {static_cast<float>(constants::SERVER_WINDOW_X) * this->_zoom,
                static_cast<float>(constants::SERVER_WINDOW_Y) * this->_zoom},
            RGBA{0, 0, 0, 0}, 3, selectionColor);
        return;
    }

    if (collider != nullptr) {
        const float width = collider->size.x * std::abs(transform->scale.x) * this->_zoom;
        const float height = collider->size.y * std::abs(transform->scale.y) * this->_zoom;
        if (collider->colliderShape == ColliderShape::Circle)
            graphic.drawCircle(hitboxTransform, width / 2.0F, RGBA{0, 0, 0, 0}, 3, selectionColor);
        else
            graphic.drawRectangle(hitboxTransform, {width, height}, RGBA{0, 0, 0, 0}, 3, selectionColor);
        return;
    }

    graphic.drawRectangle(viewTransform, {16.0F, 16.0F}, RGBA{0, 0, 0, 0}, 2, selectionColor);
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
    this->_drawSelection(graphicLib, size);
    this->_drawCamera(graphicLib, size);
}

bool OrByte::SceneView::onEvent(const OrByte::Event& event, Vector2 local)
{
    if (this->_draggedEntity) {
        if (event.type == Event::Type::MouseMoved) {
            if (Transform *transform = this->_world.getComponent<Transform>(*this->_draggedEntity)) {
                const Vector2 mouseWorld = this->_worldPositionAt(local);
                transform->position = {mouseWorld.x + this->_dragOffset.x, mouseWorld.y + this->_dragOffset.y};
            } else {
                this->_draggedEntity.reset();
            }
            return true;
        }
        if (event.type == Event::Type::MouseButtonReleased && event.mouseButton == Event::MouseButton::Left) {
            this->_draggedEntity.reset();
            return true;
        }
    }

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

    if (event.type == Event::Type::MouseButtonPressed && event.mouseButton == Event::MouseButton::Left) {
        const float areaHeight = std::max(0.0F, static_cast<float>(this->getContentRect().h) - HEADER_HEIGHT);
        if (local.y < HEADER_HEIGHT || areaHeight <= 0.0F)
            return false;
        const Vector2 worldPosition = this->_worldPositionAt(local);
        const std::vector<int> entities = this->_world.getExistingEntities();
        for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
            if (this->_hitTestEntity(*it, worldPosition)) {
                this->_world.setEntitySelected(*it);
                this->_draggedEntity = *it;
                if (const Transform *transform = this->_world.getComponent<Transform>(*it))
                    this->_dragOffset = {transform->position.x - worldPosition.x, transform->position.y - worldPosition.y};
                return true;
            }
        }
        this->_world.clearSelectedEntity();
        return true;
    }

    if (event.type != Event::Type::MouseWheelScrolled || event.wheelDelta == 0.0)
        return false;

    const float areaHeight = std::max(0.0F, static_cast<float>(this->getContentRect().h) - HEADER_HEIGHT);
    if (local.y < HEADER_HEIGHT || areaHeight <= 0.0F)
        return false;

    const Vector2 worldUnderMouse = this->_worldPositionAt(local);
    const Rect& content = this->getContentRect();
    const float centerX = static_cast<float>(content.w) / 2.0F;
    const float centerY = HEADER_HEIGHT + areaHeight / 2.0F;

    const float factor = event.wheelDelta > 0.0 ? ZOOM_FACTOR : 1.0F / ZOOM_FACTOR;
    this->_zoom = std::clamp(this->_zoom * factor, MIN_ZOOM, MAX_ZOOM);
    this->_viewCenter = {
        worldUnderMouse.x - (local.x - centerX) / this->_zoom,
        worldUnderMouse.y - (local.y - centerY) / this->_zoom,
    };
    return true;
}
