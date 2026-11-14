#include "../include/CLI.hpp"

CLI::CLI(const std::string& host, int port) : redisClient_(host, port) {}

void CLI::run() {}
