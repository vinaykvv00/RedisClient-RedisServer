#include "../include/CLI.hpp"

#include <iostream>

CLI::CLI(const std::string& host, int port) : redisClient_(host, port) {}

void CLI::run() {
    if(!redisClient_.connectToServer()) {
        std::cerr << "Failed to connect to Redis server." << std::endl;
        return;
    }

    std::cout << "Connected to Redis server." << std::endl;
    // std::string host = "127.0.0.1";
    // int port = 6379;  // default Redis port
}
