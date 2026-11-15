#include "../include/RedisClient.hpp"
// Establishing a TCP Connection to Redis(RedisClient)
// Uses Berkeley sockets to open a TCP connection to the Redis server.
// Supports IPv4 and IPv6 resolution using getaddrinfo.
// Implements:
//     connectToServer() → Establishes the connection.
//     sendCommand() → Sends a command over the socket.
//     disconnect() → Closes the socket when finished.
#include <unistd.h>  // for close()

#include <cstring>
#include <iostream>

RedisClient::RedisClient(const std::string& host, int port) : host_(host), port_(port), sockfd_(-1) {}

RedisClient::~RedisClient() {
    disconnectFromServer();
}

bool RedisClient::connectToServer() {
    struct addrinfo hints, *res = nullptr;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP socket

    std::string portStr = std::to_string(port_);                          // Convert port number to string for getaddrinfo
    int err = getaddrinfo(host_.c_str(), portStr.c_str(), &hints, &res);  // Resolve host and port into addrinfo structures
    if (err != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(err) << std::endl;
        return false;
    }

    // Attempt to connect to one of the resolved addresses in the linked list returned by getaddrinfo
    for (auto p = res; p != nullptr; p = p->ai_next) {
        sockfd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);  // create socket for this address
        if (sockfd_ == -1) continue;
        if (connect(sockfd_, p->ai_addr, p->ai_addrlen) == 0) {  // attempt to connect to this address
            break;
        }
        close(sockfd_);
        sockfd_ = -1;
    }
    freeaddrinfo(res);  // free the linked list allocated by getaddrinfo
    if (sockfd_ == -1) {
        std::cerr << "Failed to connect to " << host_ << ":" << port_ << std::endl;
    }
    return sockfd_ != -1;  // return true if connected successfully, false otherwise
}

void RedisClient::disconnectFromServer() {
    if (sockfd_ != -1) {
        close(sockfd_);
        sockfd_ = -1;
    }
}

int RedisClient::getSocketFD() const {
    return sockfd_;
}

/*commandLen = 14, totalSent = 0
iter 1: send(fd, buf+0, 14) → 14
        totalSent = 14
        loop condition: 14 < 14 → false → exit
return true
---------------
commandLen = 100000, totalSent = 0
iter 1: send(fd, buf+0,     100000) → 65536   totalSent = 65536
iter 2: send(fd, buf+65536,  34464) → 34464   totalSent = 100000
        loop: 100000 < 100000 → false → exit
return true
*/

bool RedisClient::sendCommand(const std::string& command) {
    if (sockfd_ == -1) {
        std::cerr << "Socket is not connected." << std::endl;
        return false;
    }
    ssize_t totalSent = 0;
    ssize_t commandLen = command.size();
    while (totalSent < commandLen) {
        ssize_t sent = send(sockfd_, command.c_str() + totalSent, commandLen - totalSent, 0);
        if (sent == -1) {
            std::cerr << "Failed to send command." << std::endl;
            return false;
        }
        totalSent += sent;
    }
    return true;
}
