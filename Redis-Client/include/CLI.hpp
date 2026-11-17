#ifndef CLI_HPP
#define CLI_HPP
#include <string>
#include <vector>

#include "CommandHandler.hpp"
#include "RedisClient.hpp"
#include "ResponseParser.hpp"

class CLI {
public:
    CLI(const std::string& host, int port);
    void run(const std::vector<std::string>& commandArgs);
    void executeCommand(const std::vector<std::string>& commandArgs);

private:
    RedisClient redisClient_;
    std::string host_;
    int port_;
};

#endif
