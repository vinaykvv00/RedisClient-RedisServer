#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP
#include <string>
#include <vector>

class CommandHandler {
public:
    // split the command into tokens
    static std::vector<std::string> splitArgs(const std::string& command);

private:
};

#endif  // COMMAND_HANDLER_HPP
