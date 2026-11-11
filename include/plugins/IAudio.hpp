/**
 * @file IAudio.hpp
 * @brief Audio Library Interface
 *
 * @author Thomas
 * @date 2026-11-03
**/

#pragma once
#ifndef IAUDIO_HPP_
#define IAUDIO_HPP_

#include <cstdint>
#include <filesystem>

namespace OrByte {

    enum class AudioStatus : std:: uint8_t {
        Stopped,
        Paused,
        Playing,
    };

    class IAudio {
        public:
            virtual bool load(int channel, const std::filesystem::path& path) = 0;
            virtual void unload(int channel) = 0;
            [[nodiscard]] virtual bool isLoaded(int channel) const = 0;

            virtual void play(int channel) = 0;
            virtual void pause(int channel) = 0;
            virtual void stop(int channel) = 0;
            [[nodiscard]] virtual AudioStatus getStatus(int channel) const = 0;

            virtual void setVolume(int channel, float volume) = 0;
            virtual void setPitch(int channel, float pitch) = 0;
            virtual void setLoop(int channel, bool loop) = 0;

            virtual void setOffset(int channel, float seconds) = 0;
            [[nodiscard]] virtual float getOffset(int channel) const = 0;
            [[nodiscard]] virtual float getDuration(int channel) const = 0;

            virtual void setMasterVolume(float volume) = 0;
            
            IAudio& operator=(const IAudio& object) = delete;
            IAudio& operator=(IAudio&& object) = delete;

            IAudio() = default;
            virtual ~IAudio() = default;
            IAudio(const IAudio& object) = delete;
            IAudio(IAudio&& object) = delete;
    };
} // namespace OrByte

#endif
