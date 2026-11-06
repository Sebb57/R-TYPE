/**
 * @file SFMLAudio.hpp
 * @brief IAudio implementation with SFML
 *
 * @author Thomas
 * @date 2026-11-03
**/

#pragma once
#ifndef SFMLAUDIO_HPP
#define SFMLAUDIO_HPP

#include "plugins/IAudio.hpp"
#include <SFML/Audio.hpp>
#include <memory>
#include <unordered_map>

namespace OrByte {
    class SFMLAudio final : public IAudio {
        std::unordered_map<int, std::unique_ptr<sf::Music>> _musics;
        [[nodiscard]] sf::Music *_get(int channel) const;

        public:
            SFMLAudio() = default;
            ~SFMLAudio() override;

            bool load(int channel, const std::filesystem::path& path) override;
            void unload(int channel) override;
            [[nodiscard]] bool isLoaded(int channel) const override;

            void play(int channel) override;
            void pause(int channel) override;
            void stop(int channel) override;
            [[nodiscard]] AudioStatus getStatus(int channel) const override;

            void setVolume(int channel, float volume) override;
            void setPitch(int channel, float pitch) override;
            void setLoop(int channel, bool loop) override;

            void setOffset(int channel, float seconds) override;
            [[nodiscard]] float getOffset(int channel) const override;
            [[nodiscard]] float getDuration(int channel) const override;

            void setMasterVolume(float volume) override;
    };
} //namespace Orbyte

#endif