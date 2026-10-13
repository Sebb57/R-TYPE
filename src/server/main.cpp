/**
 * @file main.cpp
 * @brief server main implementation
 *
 * @author Seb
 * @date 2026-10-05
**/

#include "Server.hpp"
#include <exception>
#include <iostream>
#include "ServerConfig.hpp"
#include "constants.hpp"

int main()
{
    try {
        server::ServerConfig const config;
        server::Server server(config);
        server.run();
        return OrByte::constants::SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << "[Fatal] " << e.what() << std::endl;
        return OrByte::constants::FAILURE;
    }
}
