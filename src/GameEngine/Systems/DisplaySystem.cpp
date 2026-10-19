/*
** EPITECH PROJECT, 2026
** RType
** File description:
** PhysicSystem.cpp
*/

#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/Systems/DisplaySystem.hpp"

void OrByte::DisplaySystem::_handleSprite(Transform* transform, int entity)
{
    Sprite* sprite = this->_world.getComponent<Sprite>(entity);
    if (sprite == nullptr)
        return;

    this->_graphic.drawImage(*transform, sprite->size, sprite->texturePath);
}

void OrByte::DisplaySystem::_handleAnimator(int entityID, Transform* transform)
{
    Animator* animator = this->_world.getComponent<Animator>(entityID);
    const Animation* animation = animator->getCurrentAnimation();

    if (!animation || animation->frameCount == 0)
        return;

    if (animation->isSpriteSheet) {
        if (animation->columns == 0 ||
            animation->frameSize.x <= 0.0f ||
            animation->frameSize.y <= 0.0f)
            return;

        const std::size_t frame = animation->firstFrame + animator->currentFrame;

        const std::size_t column = frame % animation->columns;
        const std::size_t row = frame / animation->columns;

        const int x = static_cast<int>(column * animation->frameSize.x);
        const int y = static_cast<int>(row * animation->frameSize.y);

        const Rect rect(
            x,
            y,
            static_cast<int>(animation->frameSize.x),
            static_cast<int>(animation->frameSize.y)
        );

        const Vector2 displaySize = animation->displaySize.x > 0.0f && animation->displaySize.y > 0.0f
            ? animation->displaySize : animation->frameSize;
        this->_graphic.drawImage(*transform, displaySize, animation->spriteSheetPath, rect);
    } else {
        if (animator->currentFrame >= animation->frameCount ||
            animator->currentFrame >= animation->spritePaths.size())
            return;

        const std::string& path =
            animation->spritePaths[animator->currentFrame];

        this->_graphic.drawImage(*transform, animation->frameSize, path);
    }
}

void OrByte::DisplaySystem::_updateAnimator(Animator* animator, float deltaTime)
{
    if (!animator->playing)
        return;

    const Animation* animation = animator->getCurrentAnimation();
    if (!animation || animation->frameCount == 0)
        return;

    animator->elapsed += deltaTime;

    while (animator->elapsed >= animation->frameDuration) {
        animator->elapsed -= animation->frameDuration;
        animator->currentFrame++;

        if (animator->currentFrame >= animation->frameCount) {
            if (animation->loop) {
                animator->currentFrame = 0;
            } else {
                animator->currentFrame = animation->frameCount - 1;
                animator->playing = false;
                break;
            }
        }
    }
}

void OrByte::DisplaySystem::update()
{
    std::vector<int> entities = this->_world.getExistingEntities();
    for (int entity: entities) {
        Transform* transform = this->_world.getComponent<Transform>(entity);
        if (transform == nullptr)
            continue;
        Animator* animator = this->_world.getComponent<Animator>(entity);
        if (animator != nullptr) {
            this->_updateAnimator(animator, 1.0f / constants::FPS_LIMIT);
            this->_handleAnimator(entity, transform);
        } else {
            this->_handleSprite(transform, entity);
        }
    }
}
