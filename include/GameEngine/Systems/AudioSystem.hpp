/*
** EPITECH PROJECT, 2026
** RType
** File description:
** AudioSystem.hpp
*/

#pragma once
#ifndef AUDIOSYSTEM_HPP
#define AUDIOSYSTEM_HPP

#include "GameEngine/ECS/Components/Music.hpp"
#include "GameEngine/ECS/World.hpp"
#include "plugins/ASystem.hpp"
#include "plugins/IAudio.hpp"
#include <string>
#include <unordered_map>

namespace OrByte {

class AudioSystem : public ASystem {
    struct Track {
        std::string path;
        bool loaded = false;
        bool wasPlaying = false;
    };

    World& _world;
    IAudio& _audio;
    std::unordered_map<int, Track> _tracks;

    void _updateMusic(int entity, Music& music);
    void _releaseUnused();

    public:
        AudioSystem(World& world, IAudio& audio) : _world(world), _audio(audio) {}
        ~AudioSystem() override;

        void update() override;

};

} // namespace OrByte

#endif