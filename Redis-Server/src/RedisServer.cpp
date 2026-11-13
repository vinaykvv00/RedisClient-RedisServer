#include "../include/RedisServer.hpp"
#include "../include/RedisCommandHandler.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring> // For memset
#include <thread> // For std::thread
#include <vector> // For std::vector


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

    std::vector<std::thread> threads;
    RedisCommandHandler cmdHandler;

    while(running)
    {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0)
        {
            if(running){
                std::cerr << "Failed to accept client connection!" << std::endl;
            }
        }
        else
        {
            
            threads.emplace_back([client_socket, &cmdHandler]() {
                char buffer[1024]; // Buffer for client data (if needed)
                while(true)
                {
                    memset(buffer, 0, sizeof(buffer));
                    int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (bytes <= 0) {
                        break; // Client disconnected or error occurred
                    }
                    std::string request(buffer, bytes);
                    std::string response = cmdHandler.processCommand(request);
                    send(client_socket, response.c_str(), response.size(), 0);
                }
                close(client_socket); // Close the client socket after handling the client
            });
        }
    }
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
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