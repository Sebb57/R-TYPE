/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Animation.hpp
*/

#pragma once
#ifndef ANIMATION_HPP_
    #define ANIMATION_HPP_

    #include "../Utils.hpp"
    #include "plugins/Serialization.hpp"
    #include <string>
    #include <vector>

namespace OrByte {

    struct Animation {
        std::vector<std::string> spritePaths;
        std::string spriteSheetPath;
        bool isSpriteSheet{false};

        std::size_t frameCount{1};
        std::size_t firstFrame{0};
        Vector2 frameSize{0.0f, 0.0f};
        Vector2 displaySize{0.0f, 0.0f};
        std::size_t columns{1};

        float frameDuration{0.1f};
        bool loop{true};
    };

    template <typename S>
    void serialize(S& s, Animation& animation)
    {
        s.container(animation.spritePaths, MaxContainerSize, [](S& ser, std::string& path) {
            ser.text1b(path, MaxStringSize);
        });
        s.text1b(animation.spriteSheetPath, MaxStringSize);
        s.boolValue(animation.isSpriteSheet);
        s.value8b(animation.frameCount);
        s.value8b(animation.firstFrame);
        s.object(animation.frameSize);
        s.object(animation.displaySize);
        s.value8b(animation.columns);
        s.value4b(animation.frameDuration);
        s.boolValue(animation.loop);
    }

} // namespace OrByte

#endif /* ANIMATION_HPP_ */
