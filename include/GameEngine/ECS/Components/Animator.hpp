/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Animator.hpp
*/

#pragma once
#ifndef ANIMATOR_HPP_
    #define ANIMATOR_HPP_

    #include "IComponent.hpp"
    #include "Animation.hpp"
    #include <string>
    #include <unordered_map>

namespace OrByte {

    struct Animator : public IComponent {
        inline static constexpr ComponentType Type = ComponentType::Animator;

        std::unordered_map<std::string, Animation> animations;

        std::string currentAnimation;
        std::size_t currentFrame{0};
        float elapsed{0.0f};
        bool playing{false};

        void addAnimation(const std::string& name, const Animation& animation)
        {
            animations[name] = animation;
        }

        bool play(const std::string& name, bool restart = false)
        {
            if (animations.find(name) == animations.end())
                return false;

            if (currentAnimation != name || restart) {
                currentAnimation = name;
                currentFrame = 0;
                elapsed = 0.0f;
            }

            playing = true;
            return true;
        }

        void pause()
        {
            playing = false;
        }

        void resume()
        {
            if (animations.find(currentAnimation) != animations.end())
                playing = true;
        }

        void stop()
        {
            playing = false;
            currentFrame = 0;
            elapsed = 0.0f;
        }

        const Animation* getCurrentAnimation() const
        {
            auto it = animations.find(currentAnimation);

            if (it == animations.end())
                return nullptr;

            return &it->second;
        }
    };

}

#endif /* ANIMATOR_HPP_ */
