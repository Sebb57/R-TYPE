/**
 * @file SceneContext.hpp
 * @brief everything a scene can reach from the client
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef GAMEENGINE_CLIENT_SCENES_SCENECONTEXT_HPP
#define GAMEENGINE_CLIENT_SCENES_SCENECONTEXT_HPP

#include <filesystem>
#include "GameEngine/client/ClientConfig.hpp"
#include "GameEngine/client/Controls.hpp"
#include "plugins/IGraphic.hpp"

namespace client {

class LocalGame;
class NetworkClient;
class SceneManager;

struct SceneContext {
    const ClientConfig& config;
    OrByte::IGraphic& graphic;
    LocalGame& game;
    NetworkClient& network;
    SceneManager& scenes;
    Controls& controls;
    std::filesystem::path font;
};

} // namespace client

#endif // GAMEENGINE_CLIENT_SCENES_SCENECONTEXT_HPP
