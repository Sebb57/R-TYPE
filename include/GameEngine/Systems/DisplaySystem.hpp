/*
** EPITECH PROJECT, 2026
** RType
** File description:
** DisplaySystem.hpp
*/

#pragma once
#ifndef DISPLAYSYSTEM_HPP_
    #define DISPLAYSYSTEM_HPP_

    #include "GameEngine/ECS/Components/Transform.hpp"
    #include "GameEngine/ECS/Components/Animator.hpp"
    #include "GameEngine/ECS/World.hpp"
    #include "plugins/IGraphic.hpp"
    #include <string>
    #include <unordered_map>

namespace OrByte {

class DisplaySystem {
    World& _world;
    IGraphic& _graphic;
    std::unordered_map<std::string, bool> _images;

    void _handleSprite(Transform* transform, int entity);
    void _handleAnimator(int entity, Transform* transform);
    void _updateAnimator(Animator* animator, float deltaTime);
    void _handleControllable(Transform* transform, int entity);
    bool _hasImage(const std::string& path);

    public:
        DisplaySystem(World& world, IGraphic& graphic) : _world(world), _graphic(graphic) {}
        ~DisplaySystem() = default;

        void update();
};

}

#endif /* DISPLAYSYSTEM_HPP_ */
