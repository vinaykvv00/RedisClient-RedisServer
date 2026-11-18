#ifndef REDISCLIENT_HPP
#define REDISCLIENT_HPP

#include <string>
#include <netinet/in.h>  // for sockaddr_in
#include <sys/socket.h>  // for socket functions
#include <netdb.h>       // for getaddrinfo and related structures


class RedisClient {
public:
    RedisClient(const std::string& host, int port);
    ~RedisClient();
    // Add other member functions as needed

    bool connectToServer();
    void disconnectFromServer();
    int getSocketFD() const;
    bool sendCommand(const std::string& command);

private:
    std::string host_;
    int port_;
    int sockfd_;
};

#endif
