#include "GameEngine/ECS/Entity/Ennemies.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Managers/ComponentManager.hpp"
#include "GameEngine/ECS/World.hpp"
#include <random>

RType::Ennemies::Ennemies(World *world) : RandomLine(0.0)
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

RType::Ennemies::~Ennemies()
{
}

void RType::Ennemies::CreateServeurComponent(World *world)
{
    world->addComponentToEntity<Collider>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<OrByte::Health>(this->ID, 100, 100);
    world->addComponentToEntity<OrByte::RemoveHealth>(this->ID, 10);
    world->addComponentToEntity<OrByte::Velocity>(this->ID, Vector2 {-10.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {2000.0f, this->RandomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<Owner>(this->ID, Team::Enemy);
}

void RType::Ennemies::CreateClientComponent(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {2000.0f, this->RandomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<OrByte::Sprite>(this->ID, "");
}

void RType::Ennemies::DestructEnnemies(World *world)
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
