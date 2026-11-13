#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP

#include <string>
#include <atomic>

class RedisServer {
public:
    RedisServer(int port);
    ~RedisServer();
    void run();
    void shutdown();

private:
    int port;
    int server_socket;
    std::atomic<bool> running;
};

#endif // REDIS_SERVER_HPP