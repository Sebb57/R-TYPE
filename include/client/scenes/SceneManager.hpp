/**
 * @file SceneManager.hpp
 * @brief scene registry and switching
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_SCENEMANAGER_HPP
#define CLIENT_SCENES_SCENEMANAGER_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "client/scenes/IScene.hpp"

namespace client {

class SceneManager {
    public:
        using Factory = std::function<std::unique_ptr<IScene>()>;

        void add(const std::string& name, Factory factory);
        void switchTo(const std::string& name);
        void apply();
        void onEvent(const OrByte::Event& event);
        void update(double deltaTime);
        void render();

    private:
        std::unordered_map<std::string, Factory> _factories;
        std::unique_ptr<IScene> _current;
        std::optional<std::string> _next;
};

} // namespace client

#endif // CLIENT_SCENES_SCENEMANAGER_HPP
