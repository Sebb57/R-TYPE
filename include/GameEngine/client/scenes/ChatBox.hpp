/**
 * @file ChatBox.hpp
 * @brief chat overlay of the game and spectate scenes, shows the room's chat and types new lines
 *
 * @author Seb
 * @date 2026-11-13
**/

#ifndef GAMEENGINE_CLIENT_SCENES_CHATBOX_HPP
#define GAMEENGINE_CLIENT_SCENES_CHATBOX_HPP

#ifdef ORBYTE_FEATURE_CHAT

    #include <cstddef>
    #include <cstdint>
    #include <functional>
    #include <string>
    #include <vector>
    #include "GameEngine/Event.hpp"
    #include "GameEngine/ECS/Utils.hpp"
    #include "GameEngine/client/scenes/SceneContext.hpp"
    #include "GameEngine/protocol/Chat.hpp"
    #include "plugins/IGraphic.hpp"

namespace client {

[[nodiscard]] std::vector<std::string> wrapChat(const std::string& text, std::size_t columns);

class ChatBox {
    public:
        using Send = std::function<void(std::string)>;

        ChatBox(SceneContext& context, Send send);

        bool onEvent(const OrByte::Event& event);
        void update(double deltaTime, const server::ChatLog& log);
        void render(const server::ChatLog& log) const;
        void setAvailable(bool available);
        [[nodiscard]] bool typing() const { return this->_typing; }

    private:
        void close();
        void drawText(const std::string& text, Vector2 position, unsigned size, OrByte::RGBA color, float maxWidth = 0.0F) const;
        void drawPanel(Vector2 position, Vector2 size, OrByte::RGBA fill, unsigned outlineThickness, OrByte::RGBA outline) const;

        SceneContext& _context;
        Send _send;
        std::string _input;
        bool _typing = false;
        bool _available = true;
        std::uint64_t _seen = 0;
        double _idle = 0.0;
};

} // namespace client

#endif

#endif // GAMEENGINE_CLIENT_SCENES_CHATBOX_HPP
