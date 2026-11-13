#include "GameEngine/ECS/SceneCollection.hpp"

OrByte::SceneCollection::SceneCollection()
{
    this->create();
}

void OrByte::SceneCollection::create()
{
    auto world = std::make_unique<World>();
    world->ensureCamera();
    this->_add({std::move(world), {}});
}

void OrByte::SceneCollection::_add(Scene scene)
{
    if (!this->empty())
        this->currentWorld().clearPendingEntityDrag();
    this->_scenes.push_back(std::move(scene));
    this->_selected = this->_scenes.size() - 1;
    ++this->_revision;
}

void OrByte::SceneCollection::close(std::size_t index)
{
    (void) this->_scenes.at(index);

    this->_scenes.erase(this->_scenes.begin() + index);
    ++this->_revision;
    if (this->_scenes.empty()) {
        this->_selected = 0;
        return;
    }
    if (index < this->_selected || this->_selected == this->_scenes.size())
        --this->_selected;
}

World& OrByte::SceneCollection::currentWorld() const
{
    return *this->_scenes.at(this->_selected).world;
}

void OrByte::SceneCollection::select(std::size_t index)
{
    (void) this->_scenes.at(index);

    this->currentWorld().clearPendingEntityDrag();
    if (this->_selected != index) {
        this->_selected = index;
        ++this->_revision;
    }
}

void OrByte::SceneCollection::load(const std::filesystem::path& path, const ComponentFactories& factories)
{
    // An open file already has its own world, including any unsaved edits.
    const auto normalized = std::filesystem::weakly_canonical(path);
    for (std::size_t index = 0; index < this->_scenes.size(); ++index) {
        if (!this->_scenes[index].path.empty()
            && std::filesystem::weakly_canonical(this->_scenes[index].path) == normalized) {
            this->select(index);
            return;
        }
    }

    auto world = std::make_unique<World>();
    deserializeWorld(*world, path, factories);
    // Publish only after the whole file has been read successfully.
    this->_add({std::move(world), path});
}

void OrByte::SceneCollection::save(const std::filesystem::path& path)
{
    serializeWorld(this->currentWorld(), path);
    this->_scenes.at(this->_selected).path = path;
}
