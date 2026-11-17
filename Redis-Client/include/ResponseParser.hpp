#ifndef RESPONSE_PARSER_HPP
#define RESPONSE_PARSER_HPP

#include <string>

class ResponseParser {
public:
    // Read from the given socket and parse the Redis response in string ,
    static std::string parseResponse(int sockfd);

private:
    static std::string parseSimpleString(int sockfd);
    static std::string parseSimpleError(int sockfd);
    static std::string parseInteger(int sockfd);
    static std::string parseBulkString(int sockfd);
    static std::string parseArray(int sockfd);
};

#endif  // RESPONSE_PARSER_HPP
