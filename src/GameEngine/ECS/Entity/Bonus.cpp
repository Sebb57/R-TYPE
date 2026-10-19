#include "GameEngine/ECS/Entity/Bonus.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Managers/ComponentManager.hpp"
#include "GameEngine/ECS/World.hpp"
#include <random>

RType::Bonus::Bonus(World *world)
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

RType::Bonus::~Bonus()
{
}

void RType::Bonus::CreateServeurBonus(World *world)
{
    world->addComponentToEntity<Collider>(this->ID, Vector2 {0.0f, 0.0f});
    world->addComponentToEntity<OrByte::Velocity>(this->ID, Vector2 {-5.0f, 0.0f});
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {2000.0f, this->RandomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
}

void RType::Bonus::CreateClientBonus(World *world)
{
    world->addComponentToEntity<OrByte::Transform>(this->ID, Vector2 {2000.0f, this->RandomLine}, Vector2 {1.0f, 1.0f}, float {0.0f});
    world->addComponentToEntity<OrByte::Sprite>(this->ID, "");
}
