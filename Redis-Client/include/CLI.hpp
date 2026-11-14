#ifndef CLI_HPP
#define CLI_HPP
#include <string>

#include "RedisClient.hpp"

class CLI {
public:
    CLI(const std::string& host, int port);
    void run();

private:
    RedisClient redisClient_;
};

#endif
