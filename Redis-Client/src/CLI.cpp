#include "../include/CLI.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../include/CommandHandler.hpp"

namespace {
std::string trim(const std::string& value) {
    const auto isWhitespace = [](unsigned char ch) { return std::isspace(ch) != 0; };
    const auto first = std::find_if_not(value.begin(), value.end(), isWhitespace);
    const auto last = std::find_if_not(value.rbegin(), value.rend(), isWhitespace).base();
    return first < last ? std::string(first, last) : std::string();
}
}  // anonymous namespace

CLI::CLI(const std::string& host, int port) : redisClient_(host, port) {}

void CLI::run() {
    if (!redisClient_.connectToServer()) {
        std::cerr << "Failed to connect to Redis server." << std::endl;
        return;
    }

    std::cout << "Connected to Redis server." << std::endl;
    std::cout << "Socket FD: " << redisClient_.getSocketFD() << std::endl;
    std::string host = "127.0.0.1";
    int port = 6379;  // default Redis port

    while (true) {
        std::cout << host << ":" << port << "> ";
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;
        if (line == "exit" || line == "quit") {
            std::cout << "Exiting... goodbye" << std::endl;
            break;
        }

        if (line == "help") {
            std::cout << " display help information" << std::endl;
            continue;
        }

        // split the commands into tokens;
        std::vector<std::string> args = CommandHandler::splitArgs(line);
        if (args.empty()) continue;

        for (const auto& arg : args) {
            std::cout << " args: " << arg << std::endl;
        }
    }
}
// End of CLI::run()
