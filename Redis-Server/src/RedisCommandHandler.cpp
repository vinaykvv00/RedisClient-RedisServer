#include "../include/RedisCommandHandler.hpp"

#include <charconv>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

// RESP request format: an array of bulk strings.
//   *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
//   *<number of elements>\r\n, then for each element: $<byte length>\r\n<bytes>\r\n

namespace {

// Reads a non-negative integer at `pos` that is terminated by "\r\n".
// On success stores it in `value`, moves `pos` past the "\r\n" and returns true.
bool readNumber(const std::string& input, size_t& pos, size_t& value) {
    size_t end = input.find("\r\n", pos);
    if (end == std::string::npos) return false;

    const char* first = input.data() + pos;
    const char* last = input.data() + end;
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) return false;

    pos = end + 2;
    return true;
}

}  // namespace

// Splits a client command into tokens. Accepts RESP arrays and plain
// whitespace-separated text (e.g. "GET key" typed in telnet).
// Malformed RESP input yields the tokens parsed before the error.
std::vector<std::string> parseRespCommand(const std::string& input) {
    std::vector<std::string> tokens;

    if (input.empty() || input[0] != '*') {
        std::istringstream stream(input);
        std::string token;
        while (stream >> token) tokens.push_back(token);
        return tokens;
    }

    size_t pos = 1;  // skip '*'
    size_t count;
    if (!readNumber(input, pos, count)) return tokens;

    for (size_t i = 0; i < count; i++) {
        if (pos >= input.size() || input[pos] != '$') break;
        pos++;  // skip '$'

        size_t len;
        if (!readNumber(input, pos, len)) break;

        // The data must be followed by "\r\n"; written to avoid overflow.
        if (len > input.size() - pos || input.size() - pos - len < 2) break;
        if (input.compare(pos + len, 2, "\r\n") != 0) break;

        tokens.emplace_back(input, pos, len);
        pos += len + 2;
    }
    return tokens;
}

RedisCommandHandler::RedisCommandHandler() = default;

RedisCommandHandler::~RedisCommandHandler() = default;

std::string RedisCommandHandler::processCommand(const std::string& commandLine) {
    // TODO: dispatch on parseRespCommand(commandLine)[0]
    // use resp protocal parser
    auto tokens = parseRespCommand(commandLine);
    if (tokens.empty()) return "-ERR empty command\r\n";
    for (auto& t : tokens) {
        std::cout<< t << std::endl;
    }

    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    std::ostringstream response;

    //connect to database

    //check command and arguments
    return response.str();
}
