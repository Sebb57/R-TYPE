/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AudioSystem.cpp
*/

#include "GameEngine/Systems/AudioSystem.hpp"
#include "plugins/DynamicLibrary.hpp"
#include "plugins/ISystem.hpp"
#include <vector>

OrByte::AudioSystem::~AudioSystem()
{
    for (const auto& [channel, track] : this->_tracks)
        this->_audio.unload(channel);
}

void OrByte::AudioSystem::update()
{
    for (int entity : this->_world.getExistingEntities())
        if (Music *music = this->_world.getActiveComponent<Music>(entity))
            this->_updateMusic(entity, *music);
    this->_releaseUnused();
}

void OrByte::AudioSystem::_updateMusic(int entity, Music& music)
{
    Track& track = this->_tracks[entity];

    if (track.path != music.path) {
        track = Track{ music.path, false, false };
        this->_audio.unload(entity);
        if (!music.path.empty())
            track.loaded = this->_audio.load(entity, music.path);
    }
    if (!track.loaded)
        return;

    this->_audio.setVolume(entity, music.volume);
    this->_audio.setPitch(entity, music.pitch);
    this->_audio.setLoop(entity, music.loop);

    const AudioStatus status = this->_audio.getStatus(entity);
    switch (music.state) {
        case MusicState::Playing:
            if (status == AudioStatus::Stopped && track.wasPlaying && !music.loop) {
                music.state = MusicState::Stopped;
                break;
            }
            if (status != AudioStatus::Playing)
                this->_audio.play(entity);
            break;
        case MusicState::Paused:
            if (status == AudioStatus::Playing)
                this->_audio.pause(entity);
            break;
        case MusicState::Stopped:
            if (status != AudioStatus::Stopped)
                this->_audio.stop(entity);
            break;
    }
    track.wasPlaying = music.state == MusicState::Playing;
}

void OrByte::AudioSystem::_releaseUnused()
{
    std::vector<int> unused;

    for (const auto& [entity, track] : this->_tracks)
        if (this->_world.getActiveComponent<Music>(entity) == nullptr)
            unused.push_back(entity);
    for (int entity : unused) {
        this->_audio.unload(entity);
        this->_tracks.erase(entity);
    }
}

extern "C" {
    OrByte::DynamicLibrary::PluginType type()
    {
        return OrByte::DynamicLibrary::PluginType::SYSTEMS;
    }

    OrByte::ISystem *factory(World& world, OrByte::IAudio& audio)
    {
        return new OrByte::AudioSystem(world, audio);
    }
}
