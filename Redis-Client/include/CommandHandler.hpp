#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP
#include <string>
#include <vector>

class CommandHandler {
public:
    // split the command into tokens
    static std::vector<std::string> splitArgs(const std::string& command);

    //build the RESP commands from the vector of arguments
    static std::string buildRESPCommand(const std::vector<std::string>& args);

private:
};

#endif  // COMMAND_HANDLER_HPP
