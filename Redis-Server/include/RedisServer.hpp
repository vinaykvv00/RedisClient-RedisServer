#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP

#include <atomic>
#include <string>

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

    // setup singnal for graceful shutdown ctrl+c (SIGINT)
    void setupSignalHandler();
    static void signalHandler(int signum);
    
};

#endif  // REDIS_SERVER_HPP
