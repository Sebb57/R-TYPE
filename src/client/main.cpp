/**
 * @file main.cpp
 * @brief client entry point
 *
 * @author Seb
 * @date 2026-10-13
**/

#include <exception>
#include <iostream>
#include <utility>
#include "client/Client.hpp"
#include "constants.hpp"

int main(int argc, char **argv)
{
    try {
        client::ClientConfig config;
        if (argc > 1)
            config.host = argv[1];
        if (argc > 2)
            config.username = argv[2];
        client::Client client(std::move(config));
        client.run();
        return OrByte::constants::SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr <<  e.what() << std::endl;
        return OrByte::constants::FAILURE;
    }
}
