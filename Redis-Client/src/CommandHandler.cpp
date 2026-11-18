#include "../include/CommandHandler.hpp"

#include <regex>

std::vector<std::string> CommandHandler::splitArgs(const std::string& inputs) {
    std::vector<std::string> tokens;

    // regex to match the words or quoted strings
    std::regex rgx(R"((\"[^\"]*\")|(\S+))");  // command \"arg1 with spaces\" arg2 \"arg3 with spaces\"
    std::sregex_iterator begin(inputs.begin(), inputs.end(), rgx);
    std::sregex_iterator end;
    for (auto it = begin; it != end; ++it) {
        std::string token = it->str();
        // remove the quotes
        if (token.size() >= 2 && token.front() == '\"' && token.back() == '\"') {
            token = token.substr(1, token.size() - 2);
        }
        tokens.push_back(token);
    }
    return tokens;
}

/*
 * Build a RESP command from the vector of arguments.
 * Each argument is prefixed with its length and the command is formatted according to the RESP protocol.
 * -> start an array
 * -> bulk of string
 * -> repeat for each argument
 *  //example: if args = {"SET", "key", "value"}, the RESP command will be:
    // *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
 */

std::string CommandHandler::buildRESPCommand(const std::vector<std::string>& args) {
    std::ostringstream oss;               // ostringstream means we can build the RESP command as a string stream
    oss << "*" << args.size() << "\r\n";  // start an array with the number of arguments
    for (const auto& arg : args) {
        oss << "$" << arg.size() << "\r\n" << arg << "\r\n";  // bulk string with its length and content
    }
    return oss.str();
}
