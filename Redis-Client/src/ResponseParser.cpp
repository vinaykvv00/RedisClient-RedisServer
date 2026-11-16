#include "../include/ResponseParser.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

// Function to read single character from socket
static bool readChar(int sockfd, char& c) {
    ssize_t bytesReceived = recv(sockfd, &c, 1, 0);
    return bytesReceived == 1;
}

// Read the line of text from socket untill '\r\n' is encountered
static std::string readLine(int sockfd) {
    std::string line;
    char c;
    while (readChar(sockfd, c)) {
        if (c == '\r') {
            // except '\n' next, read and break if '\n' is found
            if (readChar(sockfd, c) && c == '\n') {
                break;
            }
        }
        line += c;
    }
    return line;
}

std::string ResponseParser::parseResponse(int sockfd) {
    char prefix;
    if (!readChar(sockfd, prefix)) {
        return "(Error): No response from server or connection closed";
    }
    switch (prefix) {
        case '+':
            return parseSimpleString(sockfd);
        // case '-':
        //     return parseSimpleError(sockfd);
        // case ':':
        //     return parseInteger(sockfd);
        // case '$':
        //     return parseBulkString(sockfd);
        // case '*':
        //     return parseArray(sockfd);
        default:
            return "(Error): Unknown response prefix";
    }
}

std::string ResponseParser::parseSimpleString(int sockfd) {
    return readLine(sockfd);
}
// std::string ResponseParser::parseSimpleError(int sockfd) {}
// std::string ResponseParser::parseInteger(int sockfd) {}
// std::string ResponseParser::parseBulkString(int sockfd) {}
// std::string ResponseParser::parseArray(int sockfd) {}
