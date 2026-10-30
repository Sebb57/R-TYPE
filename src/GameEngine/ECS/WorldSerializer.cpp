/*
** EPITECH PROJECT, 2026
** RType
** File description:
** WorldSerializer.cpp
*/

#include "GameEngine/ECS/WorldSerializer.hpp"
#include "plugins/Serialization.hpp"
#include <fstream>
#include <memory>
#include <string>
#include <vector>

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
    Deserializer deserializer{file};
    std::uint32_t magic = 0;
    std::uint32_t entityCount = 0;

    deserializer.value4b(magic);
    if (deserializer.adapter().error() != bitsery::ReaderError::NoError || magic != WORLD_MAGIC)
        throw InvalidWorld(path, "is not an OrByte world");
    deserializer.value4b(entityCount);
    if (deserializer.adapter().error() != bitsery::ReaderError::NoError || entityCount > MaxContainerSize)
        throw InvalidWorld(path, "has an invalid entity count");

    world.clearWorld();
    for (std::uint32_t i = 0; i < entityCount; ++i) {
        int id = 0;
        std::uint32_t componentCount = 0;

        deserializer.value4b(id);
        deserializer.value4b(componentCount);
        if (deserializer.adapter().error() != bitsery::ReaderError::NoError || componentCount > static_cast<std::uint32_t>(ComponentType::ENUM_END))
            throw InvalidWorld(path, "has an invalid entity header");
        world.createEntity(id);
        for (std::uint32_t c = 0; c < componentCount; c++) {
            ComponentType type{};
            deserializer.value1b(type);

            auto factory = factories.find(type);
            if (factory == factories.end())
                throw MissingComponent(path, type);
            std::unique_ptr<IComponent> component{factory->second()};
            component->deserialize(deserializer);
            world.addComponentToEntity(id, std::move(component));
        }
        if (deserializer.adapter().error() != bitsery::ReaderError::NoError)
            throw InvalidWorld(path, "is truncated or corrupted");
    }
}
