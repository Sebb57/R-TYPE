/**
 * @file Controls.hpp
 * @brief rebindable key to action mapping
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_CONTROLS_HPP
#define CLIENT_CONTROLS_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include "GameEngine/Event.hpp"

namespace client {

enum class Action { Up, Down, Left, Right, Fire, Confirm, Back };

class Controls {
    public:
        Controls();

        void rebind(Action action, OrByte::Event::Key key);
        [[nodiscard]] std::optional<Action> actionOf(OrByte::Event::Key key) const;
        [[nodiscard]] OrByte::Event::Key keyOf(Action action) const;
        [[nodiscard]] std::string keyName(Action action) const;

    private:
        std::unordered_map<Action, OrByte::Event::Key> _keys;
};

} // namespace client

#endif // CLIENT_CONTROLS_HPP
