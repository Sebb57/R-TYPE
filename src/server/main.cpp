/**
 * @file main.cpp
 * @brief server main implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "Server.hpp"
#include <iostream>
#include "constants.hpp"

int main()
{
    try {
        server::ServerConfig config;
        server::Server server(config);
        server.run();
        return RType::SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << "[Fatal] " << e.what() << std::endl;
        return RType::FAILURE;
    }
}
