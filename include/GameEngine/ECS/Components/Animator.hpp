/*
** EPITECH PROJECT, 2026
** RType
** File description:
** Animator.hpp
*/

#pragma once
#ifndef ANIMATOR_HPP_
    #define ANIMATOR_HPP_

    #include "plugins/AComponent.hpp"
    #include "Animation.hpp"
    #include <bitsery/ext/std_map.h>
    #include <string>
    #include <unordered_map>

namespace OrByte {

    struct Animator : public OrByte::AComponent<OrByte::ComponentType::Animator> {

        std::unordered_map<std::string, Animation> animations;

        std::string currentAnimation;
        std::size_t currentFrame{0};
        float elapsed{0.0f};
        bool playing{true};

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

        void serialize(Serializer& serializer) const override
        {
            serializer.ext(this->animations, bitsery::ext::StdMap{MaxContainerSize},
                [](Serializer& ser, std::string& name, Animation& animation) {
                    ser.text1b(name, MaxStringSize);
                    ser.object(animation);
                });
            serializer.text1b(this->currentAnimation, MaxStringSize);
            serializer.value8b(this->currentFrame);
            serializer.value4b(this->elapsed);
            serializer.boolValue(this->playing);
        }

        void deserialize(Deserializer& deserializer) override
        {
            deserializer.ext(this->animations, bitsery::ext::StdMap{MaxContainerSize},
                [](Deserializer& des, std::string& name, Animation& animation) {
                    des.text1b(name, MaxStringSize);
                    des.object(animation);
                });
            deserializer.text1b(this->currentAnimation, MaxStringSize);
            deserializer.value8b(this->currentFrame);
            deserializer.value4b(this->elapsed);
            deserializer.boolValue(this->playing);
        }
    };

} // namespace OrByte

#endif /* ANIMATOR_HPP_ */
