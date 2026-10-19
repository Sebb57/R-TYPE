#include "Orbyte/Projectiles.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/Animation.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "constants.hpp"
#include <cmath>

RType::Projectiles::Projectiles(World *world, int shooterId, Team owner, float verticalOffset) : _owner(owner), _spawnPos{0.0f, 0.0f}, _velocity{0.0f, 0.0f}
{
    constexpr float SPAWN_OFFSET = 50.0f;
    this->ID = world->createEntity();
    const float dir = (owner == Team::Player) ? 1.0f : -1.0f;
    OrByte::Transform *shooterTransform = world->getComponent<OrByte::Transform>(shooterId);

    if (shooterTransform != nullptr) {
        this->_spawnPos.x = shooterTransform->position.x + dir * SPAWN_OFFSET;
        this->_spawnPos.y = shooterTransform->position.y + verticalOffset;
    }

    this->_velocity.x = dir * OrByte::constants::PROJECTILE_SPEED;
    this->_velocity.y = 0.0f;
}

void RType::Projectiles::CreateServeurComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->ID, Vector2(OrByte::constants::PROJECTILE_WIDTH * OrByte::constants::PROJECTILE_COLLIDER_SCALE,
        OrByte::constants::PROJECTILE_HEIGHT * OrByte::constants::PROJECTILE_COLLIDER_SCALE));
    world->addComponentToEntity<OrByte::Velocity>(this->ID, this->_velocity);
    world->addComponentToEntity<OrByte::Transform>(this->ID, this->_spawnPos, Vector2 {1.0f, 1.0f}, 0.0f);
    world->addComponentToEntity<OrByte::RemoveHealth>(this->ID, 10);
    world->addComponentToEntity<Owner>(this->ID, _owner);
}

void RType::Projectiles::CreateClientComponent(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->ID, this->_spawnPos, Vector2 {1.0f, 1.0f}, 0.0f);
    OrByte::Animation animation;
    animation.spriteSheetPath = "./assets/sprites/bullet_1.png";
    animation.isSpriteSheet = true;
    animation.frameCount = 2;
    animation.frameSize = Vector2(OrByte::constants::PROJECTILE_FRAME_WIDTH, OrByte::constants::PROJECTILE_FRAME_HEIGHT);
    animation.displaySize = Vector2(OrByte::constants::PROJECTILE_WIDTH, OrByte::constants::PROJECTILE_HEIGHT);
    animation.columns = 2;
    animation.frameDuration = 0.08f;
    animation.loop = false;

    OrByte::Animator animator;
    animator.addAnimation("bullet", animation);
    animator.play("bullet");
    world->addComponentToEntity<OrByte::Animator>(this->ID, animator);
}

static bool applyDamage(World *world, int attacker, int victim)
{
    OrByte::RemoveHealth *damage = world->getComponent<OrByte::RemoveHealth>(attacker);
    OrByte::Health       *health = world->getComponent<OrByte::Health>(victim);

    if (damage == nullptr || health == nullptr)
        return false;

    health->currentHealth -= damage->hitDamage;
    return health->currentHealth <= 0;
}

void RType::Projectiles::CheckCollision(World *world, Vector2 entity)
{
    const int idA = static_cast<int>(entity.x);
    const int idB = static_cast<int>(entity.y);

    Owner *ownerA = world->getComponent<Owner>(idA);
    Owner *ownerB = world->getComponent<Owner>(idB);
    if (ownerA && ownerB && ownerA->team == ownerB->team)
        return;

    applyDamage(world, idB, idA);
    applyDamage(world, idA, idB);
}
