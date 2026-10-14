#include "GameEngine/ECS/Entity/Playeur.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"

RType::Playeur::Playeur(World *world)
{
    this->ID = world->createEntity();
}

RType::Playeur::~Playeur()
{
}

void RType::Playeur::CreateServeurComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Health>(this->ID, 100, 100);
    world->addComponentToEntity<Velocity>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
}

void RType::Playeur::CreateClientComponent(World *world)
{
    world->addComponentToEntity<Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Sprite>(this->ID, "");
}
