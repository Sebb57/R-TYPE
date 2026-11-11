#include "Orbyte/Bonus.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Animation.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/World.hpp"
#include "Orbyte/Playeur.hpp"
#include "constants.hpp"
#include <random>

OrByte::Bonus::Bonus(World *world, bool extraShot) : ExtraShot(extraShot)
{
    this->ID = world->createEntity();
    constexpr int LINE_LENGTH = 1980;
    constexpr int SLOTS = 4;
    constexpr int SLOT_SIZE = LINE_LENGTH / SLOTS;

    static std::linear_congruential_engine<std::uint_fast32_t, 16807, 0, 2147483647>
        engine{std::random_device{}()};
    std::uniform_int_distribution<int> slot(0, SLOTS - 1);
    this->RandomLine = slot(engine) * SLOT_SIZE;
}

void OrByte::Bonus::CreateServeurBonus(World *world, Vector2 position)
{
    if (position.x == 0.0f && position.y == 0.0f)
        position = Vector2 {2000.0f, this->RandomLine};
    world->addComponentToEntity<OrByte::Collider>(this->ID, Vector2 {OrByte::constants::BONUS_COLLIDER_WIDTH, OrByte::constants::BONUS_COLLIDER_HEIGHT});
    world->addComponentToEntity<OrByte::Velocity>(this->ID, Vector2 {-150.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->ID, position, Vector2 {1.0f, 1.0f}, float {0.0f});
}

void OrByte::Bonus::CreateClientBonus(World *world, bool extraShot)
{
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {2000.0f, this->RandomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    OrByte::Animation animation;
    animation.spriteSheetPath = extraShot ? "./assets/sprites/bonus_projectile.png" : "./assets/sprites/bonus_soin.png";
    animation.isSpriteSheet = true;
    animation.frameCount = 8;
    animation.frameSize = Vector2(OrByte::constants::BONUS_FRAME_SIZE, OrByte::constants::BONUS_FRAME_SIZE);
    animation.displaySize = Vector2(OrByte::constants::BONUS_WIDTH, OrByte::constants::BONUS_HEIGHT);
    animation.columns = 8;
    animation.frameDuration = 0.12f;
    animation.loop = true;

    OrByte::Animator animator;
    animator.addAnimation("bonus", animation);
    animator.play("bonus");
    world->addComponentToEntity<OrByte::Animator>(this->ID, animator);
}

void OrByte::Bonus::ApplyTo(World *world, Playeur& player) const
{
    player.ApplyBonus(world, this->ExtraShot);
}
