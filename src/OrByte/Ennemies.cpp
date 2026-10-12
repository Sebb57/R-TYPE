#include "../../include/OrByte/Ennemies.hpp"
#include "../../include/GameEngine/ECS/Components/Collider.hpp"
#include "../../include/GameEngine/ECS/Components/Health.hpp"
#include "../../include/GameEngine/ECS/Components/Sprite.hpp"
#include "../../include/GameEngine/ECS/Components/Transform.hpp"
#include "../../include/GameEngine/ECS/Components/Velocity.hpp"

RType::Ennemies::Ennemies(World *world)
{
    this->ID = world->createEntity();
}

RType::Ennemies::~Ennemies()
{
}

void RType::Ennemies::CreateServeurComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Health>(this->ID, 100, 100);
    world->addComponentToEntity<Velocity>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
}

void RType::Ennemies::CreateClientComponent(World *world)
{
    world->addComponentToEntity<Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Sprite>(this->ID, "");
}
