/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Music.hpp
*/

#pragma once
#ifndef MUSIC_HPP_
    #define MUSIC_HPP_

    #include "plugins/AComponent.hpp"
    #include <cstdint>
    #include <string>
    #include <utility>

namespace OrByte {

enum class MusicState : std::uint8_t {
    Playing,
    Paused,
    Stopped,
};

struct Music : public OrByte::AComponent<OrByte::ComponentType::Music>
{
    Music() = default;
    Music(std::string path, float volume = 100.F, bool loop = true)
        : path(std::move(path)), volume(volume), loop(loop) {}

    std::string path;
    float volume{100.F};
    float pitch{1.F};
    bool loop{true};
    MusicState state{MusicState::Playing};

    void serialize(Serializer& serializer) const final
    {
        serializer.text1b(this->path, MaxStringSize);
        serializer.value4b(this->volume);
        serializer.value4b(this->pitch);
        serializer.boolValue(this->loop);
        serializer.value1b(this->state);
    }
    void deserialize(Deserializer& deserializer) final
    {
        deserializer.text1b(this->path, MaxStringSize);
        deserializer.value4b(this->volume);
        deserializer.value4b(this->pitch);
        deserializer.boolValue(this->loop);
        deserializer.value1b(this->state);
    }
};

} // namespace OrByte

#endif /* MUSIC_HPP_ */
