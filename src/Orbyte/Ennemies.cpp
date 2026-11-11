#include "Orbyte/Ennemies.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/Animation.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/World.hpp"
#include "constants.hpp"
#include <random>

OrByte::Ennemies::Ennemies(World *world) : RandomLine(0.0)
{
    this->ID = world->createEntity();
    thread_local std::mt19937 engine(std::random_device{}());
    std::uniform_real_distribution<float> line(
        OrByte::constants::ENEMY_HEIGHT / 2.0f,
        OrByte::constants::SERVER_WINDOW_Y - OrByte::constants::ENEMY_HEIGHT / 2.0f
    );
    this->RandomLine = line(engine);
}

bool OrByte::Ennemies::ShouldDropBonus()
{
    thread_local std::mt19937 engine(std::random_device{}());
    return std::uniform_int_distribution<unsigned>(1, 8)(engine) == 1;
}

void OrByte::Ennemies::CreateServeurComponent(World *world)
{
    world->addComponentToEntity<OrByte::Collider>(this->ID, Vector2(OrByte::constants::ENEMY_COLLIDER_WIDTH, OrByte::constants::ENEMY_COLLIDER_HEIGHT));
    world->addComponentToEntity<OrByte::Health>(this->ID, 50, 50);
    world->addComponentToEntity<OrByte::Velocity>(this->ID, Vector2 {-320.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {2000.0f, this->RandomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<OrByte::Owner>(this->ID, OrByte::Team::Enemy);
}

void OrByte::Ennemies::CreateClientComponent(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {2000.0f, this->RandomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    OrByte::Animation animation;
    animation.spriteSheetPath = "./assets/sprites/ship_ennemy.png";
    animation.isSpriteSheet = true;
    animation.frameCount = 8;
    animation.frameSize = Vector2(OrByte::constants::ENEMY_FRAME_WIDTH, OrByte::constants::ENEMY_FRAME_HEIGHT);
    animation.displaySize = Vector2(OrByte::constants::ENEMY_WIDTH, OrByte::constants::ENEMY_HEIGHT);
    animation.columns = 8;
    animation.frameDuration = 0.1f;
    animation.loop = true;

    OrByte::Animator animator;
    animator.addAnimation("enemy", animation);
    animator.play("enemy");
    world->addComponentToEntity<OrByte::Animator>(this->ID, animator);
}

void OrByte::Ennemies::DestructEnnemies(World *world)
{
    constexpr float OFFSCREEN_X = -100.0f;
    std::vector<int> entities = world->getExistingEntities();
    for (int id : entities) {
        OrByte::Transform *transform = world->getComponent<OrByte::Transform>(id);
        OrByte::Health *health = world->getComponent<OrByte::Health>(id);
        if (transform == nullptr || health == nullptr)
            continue;
        if (transform->position.x < OFFSCREEN_X)
            world->destroyEntity(id);
    }
}
