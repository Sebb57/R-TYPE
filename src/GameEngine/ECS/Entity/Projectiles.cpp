#include "GameEngine/ECS/Entity/Projectiles.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"

RType::Projectiles::Projectiles(World *world)
{
    this->ID = world->createEntity();
}

RType::Projectiles::~Projectiles()
{
}

void RType::Projectiles::CreateServeurComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Velocity>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
}

void RType::Projectiles::CreateClientComponent(World *world)
{
    world->addComponentToEntity<Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Sprite>(this->ID, "");
}
