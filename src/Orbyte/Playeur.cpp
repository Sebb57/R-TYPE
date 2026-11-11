#include "Orbyte/Playeur.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/Animation.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "Orbyte/Projectiles.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cmath>

OrByte::Playeur::Playeur(World *world)
{
    this->ID = world->createEntity();
}

void OrByte::Playeur::CreateServeurComponent(World *world, Vector2 position, std::uint8_t playerSlot)
{
    const std::size_t slot = playerSlot < OrByte::constants::PLAYER_FRAME_WIDTHS.size() ? playerSlot : 0;
    const Vector2 size(OrByte::constants::playerWidthForSlot(static_cast<unsigned>(slot)), OrByte::constants::PLAYER_HEIGHT);
    world->addComponentToEntity<Collider>(this->ID, Vector2(size.x * OrByte::constants::PLAYER_COLLIDER_SCALE,
        size.y * OrByte::constants::PLAYER_COLLIDER_SCALE));
    world->addComponentToEntity<OrByte::Health>(this->ID, 100, 100);
    world->addComponentToEntity<OrByte::Velocity>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->ID, position, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<OrByte::Owner>(this->ID, OrByte::Team::Player);
}

void OrByte::Playeur::CreateClientComponent(World *world, std::uint8_t playerSlot)
{
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    constexpr const char *shipPaths[] = {
        "./assets/sprites/ship_1.png",
        "./assets/sprites/ship_2.png",
        "./assets/sprites/ship_3.png",
        "./assets/sprites/ship_4.png",
    };
    const std::size_t slot = playerSlot < 4 ? playerSlot : 0;

    const auto makeAnimation = [&shipPaths, slot](std::size_t firstFrame, std::size_t frameCount) {
        OrByte::Animation animation;
        animation.spriteSheetPath = shipPaths[slot];
        animation.isSpriteSheet = true;
        animation.firstFrame = firstFrame;
        animation.frameCount = frameCount;
        animation.frameSize = Vector2(OrByte::constants::PLAYER_FRAME_WIDTHS[slot], OrByte::constants::PLAYER_FRAME_HEIGHTS[slot]);
        animation.displaySize = Vector2(OrByte::constants::playerWidthForSlot(static_cast<unsigned>(slot)), OrByte::constants::PLAYER_HEIGHT);
        animation.columns = 5;
        animation.frameDuration = 0.12f;
        animation.loop = false;
        return animation;
    };

    OrByte::Animator animator;
    animator.addAnimation("movedown", makeAnimation(0, 2));
    animator.addAnimation("moveforward", makeAnimation(2, 1));
    animator.addAnimation("moveup", makeAnimation(3, 2));
    animator.play("moveforward");
    world->addComponentToEntity<OrByte::Animator>(this->ID, animator);
}

void OrByte::Playeur::Steer(World *world, Vector2 direction)
{
    auto *velocity = world->getComponent<OrByte::Velocity>(this->ID);
    if (velocity == nullptr)
        return;
    const float length = std::hypot(direction.x, direction.y);
    const float speed = length > 0.0f ? OrByte::constants::PLAYER_SPEED / length : 0.0f;
    velocity->velocity = Vector2(direction.x * speed, direction.y * speed);
}

std::vector<int> OrByte::Playeur::Fire(World *world)
{
    std::vector<int> projectiles;
    for (unsigned index = 0; index < this->ProjectileCount; ++index) {
        const float offset = (static_cast<float>(index) - (this->ProjectileCount - 1) / 2.0f) * 18.0f;
        OrByte::Projectiles projectile(world, this->ID, OrByte::Team::Player, offset);
        projectile.CreateServeurComponent(world);
        projectiles.push_back(projectile.GetID());
    }
    return projectiles;
}

void OrByte::Playeur::ApplyBonus(World *world, bool extraShot)
{
    if (extraShot) {
        this->ProjectileCount = std::min(this->ProjectileCount + 1, 3U);
        return;
    }
    if (auto *health = world->getComponent<OrByte::Health>(this->ID))
        health->currentHealth = std::min(health->maxHealth, health->currentHealth + 25);
}

void OrByte::Playeur::ReceiveEnemyHit(World *world, int damage)
{
    if (auto *health = world->getComponent<OrByte::Health>(this->ID))
        health->currentHealth -= damage;
    if (this->ProjectileCount > 1)
        --this->ProjectileCount;
}

void OrByte::Playeur::KeepInBounds(World *world, float width, float height)
{
    auto *transform = world->getComponent<OrByte::Transform>(this->ID);
    const auto *collider = world->getComponent<Collider>(this->ID);
    if (transform == nullptr || collider == nullptr)
        return;
    transform->position.x = std::clamp(transform->position.x, collider->size.x / 2.0f, width - collider->size.x / 2.0f);
    transform->position.y = std::clamp(transform->position.y, collider->size.y / 2.0f, height - collider->size.y / 2.0f);
}
