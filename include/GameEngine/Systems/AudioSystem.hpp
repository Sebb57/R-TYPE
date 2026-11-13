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
#include "plugins/IGraphic.hpp"
#include <optional>
#include <string>

namespace OrByte {

class AudioSystem : public ASystem {
    struct Track {
        int entity;
        std::string path;
        float volume;
        bool loop;
    };

    World& _world;
    IGraphic& _graphic;
    std::optional<Track> _track;

    [[nodiscard]] bool _keepTrack();
    void _play(int entity, const Music& music);

    public:
        AudioSystem(World& world, IGraphic& graphic) : _world(world), _graphic(graphic) {}
        ~AudioSystem() override;

        void update() override;

};

} // namespace OrByte

#endif
