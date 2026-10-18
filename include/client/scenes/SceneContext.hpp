/**
 * @file SceneContext.hpp
 * @brief everything a scene can reach from the client
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_SCENES_SCENECONTEXT_HPP
#define CLIENT_SCENES_SCENECONTEXT_HPP

#include <filesystem>
#include "client/ClientConfig.hpp"
#include "client/Controls.hpp"
#include "plugins/IGraphic.hpp"

namespace client {

class NetworkClient;
class SceneManager;

struct SceneContext {
    const ClientConfig& config;
    OrByte::IGraphic& graphic;
    NetworkClient& network;
    SceneManager& scenes;
    Controls& controls;
    std::filesystem::path font;
};

} // namespace client

#endif // CLIENT_SCENES_SCENECONTEXT_HPP
