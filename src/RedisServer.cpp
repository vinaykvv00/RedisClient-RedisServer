#include "../include/RedisServer.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>


static RedisServer *globalServer = nullptr;

RedisServer::RedisServer(int port) : port(port), server_socket(-1), running(true) {
    globalServer = this;
}

void RedisServer::run() {

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create server socket!" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))== -1)
    {
        std::cerr << "Failed to bind server socket!" << std::endl;
        return;
    }

    if (listen(server_socket, 10) < 0)
    {
        std::cerr << "Failed to listen on server socket!" << std::endl;
        return;
    }

    std::cout << "Server is running and listening on port " << port << std::endl;
}

void RedisServer::shutdown() {
    running = false;
    if(server_socket != -1) {
        close(server_socket);
    }
    std::cout<<"Server shutdown completed!."<<std::endl;
}
RedisServer::~RedisServer() {
    shutdown();
    globalServer = nullptr;
}