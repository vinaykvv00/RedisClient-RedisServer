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

CLI::CLI(const std::string& host, int port) : redisClient_(host, port), host_(host), port_(port) {}

void CLI::executeCommand(const std::vector<std::string>& commandArgs) {
    if (commandArgs.empty()) return;

    std::string command = CommandHandler::buildRESPCommand(commandArgs);
    if (!redisClient_.sendCommand(command)) {
        std::cerr << "Failed to send command." << std::endl;
        return;
    }
    std::string response = ResponseParser::parseResponse(redisClient_.getSocketFD());
    std::cout << response << std::endl;
}  // End of CLI::executeCommand()

void CLI::run(const std::vector<std::string>& commandArgs) {
    if (!redisClient_.connectToServer()) {
        std::cerr << "Failed to connect to Redis server." << std::endl;
        return;
    }

    std::cout << "Connected to Redis server." << std::endl;
    std::cout << "Socket FD: " << redisClient_.getSocketFD() << std::endl;

    if (!commandArgs.empty()) {
        executeCommand(commandArgs);
        return;
    }

    while (true) {
        std::cout << host_ << ":" << port_ << "> ";
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

        // for (const auto& arg : args) {
        //     std::cout << " args: " << arg << std::endl;
        // }

        std::string command = CommandHandler::buildRESPCommand(args);
        if (!redisClient_.sendCommand(command)) {
            std::cerr << "Failed to send command." << std::endl;
        }
        // parse and print the message
        std::string response = ResponseParser::parseResponse(redisClient_.getSocketFD());
        std::cout << response << std::endl;
    }
}
// End of CLI::run()
