#ifndef REDIS_COMMAND_HANDLER_HPP
#define REDIS_COMMAND_HANDLER_HPP

#include <string>

class RedisCommandHandler {
public:
    RedisCommandHandler();
    //process the commands from clients and returns RESP formatted response.
    std::string processCommand(const std::string &commandLine);

    ~RedisCommandHandler();
};

#endif // REDIS_COMMAND_HANDLER_HPP