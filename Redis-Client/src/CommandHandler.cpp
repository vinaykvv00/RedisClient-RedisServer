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
