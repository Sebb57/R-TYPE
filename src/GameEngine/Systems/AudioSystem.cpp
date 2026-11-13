/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AudioSystem.cpp
*/

#include "GameEngine/Systems/AudioSystem.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"

OrByte::AudioSystem::~AudioSystem()
{
    if (this->_track)
        this->_graphic.stopMusic();
}

void OrByte::AudioSystem::update()
{
    if (this->_keepTrack())
        return;
    for (int entity : this->_world.getExistingEntities()) {
        const Music *music = this->_world.getActiveComponent<Music>(entity);

        if (music != nullptr && music->state == MusicState::Playing && !music->path.empty()) {
            this->_play(entity, *music);
            return;
        }
    }
}

bool OrByte::AudioSystem::_keepTrack()
{
    if (!this->_track)
        return false;

    Music *music = this->_world.getActiveComponent<Music>(this->_track->entity);

    if (music == nullptr || music->state != MusicState::Playing || music->path != this->_track->path) {
        this->_graphic.stopMusic();
        this->_track.reset();
        return false;
    }
    if (!this->_graphic.isMusicPlaying()) {
        music->state = MusicState::Stopped;
        this->_track.reset();
        return false;
    }
    if (music->volume != this->_track->volume || music->loop != this->_track->loop)
        this->_play(this->_track->entity, *music);
    return true;
}

void OrByte::AudioSystem::_play(int entity, const Music& music)
{
    this->_graphic.playMusic(music.path, music.loop, music.volume);
    this->_track = Track{entity, music.path, music.volume, music.loop};
}

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::SYSTEMS;
    }

    OrByte::ISystem *factory(World& world, OrByte::IGraphic& graphic)
    {
        return new OrByte::AudioSystem(world, graphic);
    }
}
