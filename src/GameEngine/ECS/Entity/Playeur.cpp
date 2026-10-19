#include "GameEngine/ECS/Entity/Playeur.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"

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
    world->addComponentToEntity<OrByte::Health>(this->ID, 100, 100);
    world->addComponentToEntity<OrByte::Velocity>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Owner>(this->ID, Team::Player);
}

void RType::Playeur::CreateClientComponent(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {0.0f, 0.0f}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<OrByte::Sprite>(this->ID, "");
}
