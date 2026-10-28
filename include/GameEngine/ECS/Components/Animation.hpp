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

}

#endif /* ANIMATION_HPP_ */
