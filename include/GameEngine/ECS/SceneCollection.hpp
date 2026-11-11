#pragma once

#include "GameEngine/ECS/WorldSerializer.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>

namespace OrByte {

// Each editor scene owns an independent world.
class SceneCollection {
    public:
        struct Scene {
            std::unique_ptr<World> world;
            std::filesystem::path path;
        };

        SceneCollection();
        [[nodiscard]] bool empty() const { return this->_scenes.empty(); }
        [[nodiscard]] World& currentWorld() const;
        [[nodiscard]] const std::vector<Scene>& scenes() const { return this->_scenes; }
        [[nodiscard]] std::size_t selectedIndex() const { return this->_selected; }
        [[nodiscard]] std::size_t revision() const { return this->_revision; }
        void create();
        void close(std::size_t index);
        void select(std::size_t index);
        void load(const std::filesystem::path& path, const ComponentFactories& factories);
        void save(const std::filesystem::path& path);

    private:
        std::vector<Scene> _scenes;
        std::size_t _selected = 0;
        std::size_t _revision = 0;

        void _add(Scene scene);
};

} // namespace OrByte
