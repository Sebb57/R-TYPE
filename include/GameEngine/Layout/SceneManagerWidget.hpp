#pragma once

#include "GameEngine/ECS/SceneCollection.hpp"
#include "GameEngine/Layout/Panel.hpp"

namespace OrByte {

class SceneManagerWidget final : public Panel {
    public:
        explicit SceneManagerWidget(SceneCollection& scenes);

    protected:
        void draw(const IGraphic& graphic, Vector2u size) override;
        bool onEvent(const Event& event, Vector2 localMouse) override;

    private:
        SceneCollection& _scenes;
        std::size_t _scrollOffset = 0;
        std::size_t _lastSelected = static_cast<std::size_t>(-1);
        unsigned _lastWidth = 0;
        void _revealSelection(std::size_t visibleTabs);
};

} // namespace OrByte
