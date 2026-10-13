/**
 * @file ClientConfig.hpp
 * @brief client config struct
 *
 * @author Seb
 * @date 2026-10-13
**/

#ifndef CLIENT_CLIENTCONFIG_HPP
#define CLIENT_CLIENTCONFIG_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include "constants.hpp"

namespace client {

struct ClientConfig {
    std::string host = "127.0.0.1";
    std::string username = "Player";
    std::uint16_t tcpPort = 6969;
    unsigned width = 1280;
    unsigned height = 720;
    std::string title = "R-Type";
    std::filesystem::path font = OrByte::constants::FONT_DEFAULT;
};

} // namespace client

#endif // CLIENT_CLIENTCONFIG_HPP
