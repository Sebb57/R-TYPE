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
    #include "plugins/ASystem.hpp"
    #include "plugins/IGraphic.hpp"

namespace OrByte {

class DisplaySystem : public ASystem {
    World& _world;
    IGraphic& _graphic;

    void _handleSprite(Transform* transform, int entity);
    void _handleAnimator(int entity, Transform* transform);
    void _updateAnimator(Animator* animator, float deltaTime);

    public:
        DisplaySystem(World& world, IGraphic& graphic) : _world(world), _graphic(graphic) {}
        ~DisplaySystem() override = default;

        void update() override;
};

} // namespace OrByte

#endif /* DISPLAYSYSTEM_HPP_ */
