/*
** EPITECH PROJECT, 2026
** RType
** File description:
** WorldSerializer.cpp
*/

#include "GameEngine/ECS/WorldSerializer.hpp"
#include "plugins/Serialization.hpp"
#include "GameEngine/ECS/Components/AddHealth.hpp"
#include "GameEngine/ECS/Components/Animator.hpp"
#include "GameEngine/ECS/Components/Camera.hpp"
#include "GameEngine/ECS/Components/Collider.hpp"
#include "GameEngine/ECS/Components/Controllable.hpp"
#include "GameEngine/ECS/Components/Health.hpp"
#include "GameEngine/ECS/Components/Music.hpp"
#include "GameEngine/ECS/Components/Name.hpp"
#include "GameEngine/ECS/Components/Orbit.hpp"
#include "GameEngine/ECS/Components/Owner.hpp"
#include "GameEngine/ECS/Components/Poison.hpp"
#include "GameEngine/ECS/Components/RegenHealth.hpp"
#include "GameEngine/ECS/Components/RemoveHealth.hpp"
#include "GameEngine/ECS/Components/Script.hpp"
#include "GameEngine/ECS/Components/Sinus.hpp"
#include "GameEngine/ECS/Components/Spiral.hpp"
#include "GameEngine/ECS/Components/Sprite.hpp"
#include "GameEngine/ECS/Components/Transform.hpp"
#include "GameEngine/ECS/Components/Velocity.hpp"
#include "GameEngine/ECS/Components/Wither.hpp"
#include <fstream>
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace {

// Read disabled components too: .obe payloads have no per-component byte length.
// These native readers require no component library and preserve stream alignment.
template <typename... Components>
OrByte::ComponentFactories nativeReaders()
{
    return {{Components::Type, +[]() -> OrByte::IComponent* { return new Components; }}...};
}

const auto readers = nativeReaders<
    OrByte::AddHealth, OrByte::Animator, OrByte::Camera, OrByte::Collider,
    OrByte::Controllable, OrByte::Health, OrByte::Music, OrByte::Name,
    OrByte::Orbit, OrByte::Owner, OrByte::Poison, OrByte::RegenHealth,
    OrByte::RemoveHealth, OrByte::Script, OrByte::Sinus, OrByte::Spiral,
    OrByte::Sprite, OrByte::Transform, OrByte::Velocity, OrByte::Wither>();

} // namespace

void OrByte::serializeWorld(const World& world, const std::filesystem::path& path)
{
    std::fstream file{path, std::ios::out | std::ios::binary | std::ios::trunc};
    if (!file.is_open())
        throw WorldOpenFailed(path);
    Serializer serializer{file};
    const std::vector<int> entities = world.getExistingEntities();

    serializer.value4b(WORLD_MAGIC);
    serializer.value4b(static_cast<std::uint32_t>(entities.size()));
    for (int id : entities) {
        const std::vector<IComponent*> components = world.getComponents(id);

        serializer.value4b(id);
        serializer.value4b(static_cast<std::uint32_t>(components.size()));
        for (const auto* component : components) {
            serializer.value1b(component->getType());
            component->serialize(serializer);
        }
    }
    serializer.adapter().flush();
    if (!file)
        throw WorldWriteFailed(path);
}

void OrByte::deserializeWorld(World& world, const std::filesystem::path& path, const ComponentFactories& factories)
{
    std::fstream file{path, std::ios::in | std::ios::binary};
    if (!file.is_open())
        throw WorldOpenFailed(path);
    deserializeWorld(world, file, factories, path);
}

void OrByte::deserializeWorld(World& world, std::istream& stream, const ComponentFactories& factories, const std::filesystem::path& source)
{
    Deserializer deserializer{stream};
    std::uint32_t magic = 0;
    std::uint32_t entityCount = 0;

    deserializer.value4b(magic);
    if (deserializer.adapter().error() != bitsery::ReaderError::NoError || magic != WORLD_MAGIC)
        throw InvalidWorld(source, "is not an OrByte world");
    deserializer.value4b(entityCount);
    if (deserializer.adapter().error() != bitsery::ReaderError::NoError || entityCount > MaxContainerSize)
        throw InvalidWorld(source, "has an invalid entity count");

    world.clearWorld();
    for (std::uint32_t i = 0; i < entityCount; ++i) {
        int id = 0;
        std::uint32_t componentCount = 0;

        deserializer.value4b(id);
        deserializer.value4b(componentCount);
        if (deserializer.adapter().error() != bitsery::ReaderError::NoError || componentCount > static_cast<std::uint32_t>(ComponentType::ENUM_END))
            throw InvalidWorld(source, "has an invalid entity header");
        world.createEntity(id);
        for (std::uint32_t c = 0; c < componentCount; c++) {
            ComponentType type{};
            deserializer.value1b(type);

            const auto factory = factories.find(type);
            const auto reader = readers.find(type);
            if (factory == factories.end() && reader == readers.end())
                throw MissingComponent(source, type);
            std::unique_ptr<IComponent> component{factory != factories.end() ? factory->second() : reader->second()};
            component->deserialize(deserializer);
            world.addComponentToEntity(id, std::move(component));
        }
        if (deserializer.adapter().error() != bitsery::ReaderError::NoError)
            throw InvalidWorld(source, "is truncated or corrupted");
    }
    world.ensureCamera();
}
