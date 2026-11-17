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
        case '-':
            return parseSimpleError(sockfd);
        case ':':
            return parseInteger(sockfd);
        case '$':
            return parseBulkString(sockfd);
        case '*':
            return parseArray(sockfd);
        default:
            return "(Error): Unknown response prefix";
    }
}
/* Trace one example of parsing a simple string response:
 * +OK\r\n
 * The function will read the line "OK" and return it.
 */
std::string ResponseParser::parseSimpleString(int sockfd) {
    return readLine(sockfd);
}

/* Trace one example of parsing a simple error response:
 * -ERR unknown command\r\n
 * The function will read the line "ERR unknown command" and return it prefixed with "(Error): ".
 */
std::string ResponseParser::parseSimpleError(int sockfd) {
    return "(Error): " + readLine(sockfd);
}

/* Trace one example of parsing an integer response:
 * :1000\r\n
 * The function will read the line "1000" and return it.
 */
std::string ResponseParser::parseInteger(int sockfd) {
    return readLine(sockfd);
}


/* Trace one example of parsing a bulk string response:
 * $6\r\nfoobar\r\n
 * The function will read the length (6), then read "foobar", and finally consume the trailing "\r\n".
 * For example, if the response is "$6\r\nfoobar\r\n", the function will return "foobar".
 */
std::string ResponseParser::parseBulkString(int sockfd) {
    std::string lenstr = readLine(sockfd);
    int len = std::stoi(lenstr);
    if (len == -1) {
        return "(nil)";
    }
    std::string result;
    result.resize(len);
    int totalRead = 0;
    while (totalRead < len) {
        ssize_t bytesReceived = recv(sockfd, &result[totalRead], len - totalRead, 0);
        if (bytesReceived <= 0) {
            return "(Error): Failed to read bulk string";
        }
        totalRead += bytesReceived;
    }
    // read the trailing "\r\n"
    char c;
    readChar(sockfd, c);
    readChar(sockfd, c);
    return result;
}

/* Trace one example of parsing an array response:
 * *2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n
 * The function will read the array length (2), then parse each element as a bulk string.
 * For example, if the response is "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n", the function will return:
 * 1) foo
 * 2) bar
 */

std::string ResponseParser::parseArray(int sockfd) {
    std::string lenstr = readLine(sockfd);
    int len = std::stoi(lenstr);
    if (len == -1) {
        return "(nil)";
    }
    std::vector<std::string> elements;
    for (int i = 0; i < len; ++i) {
        elements.push_back(parseResponse(sockfd));
    }
    std::ostringstream oss;
    for (size_t i = 0; i < elements.size(); ++i) {
        oss << i + 1 << ") " << elements[i] << "\n";
    }
    return oss.str();
}
