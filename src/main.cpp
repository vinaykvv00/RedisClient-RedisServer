#include "../include/RedisServer.hpp"
#include <iostream>
#include <string>
#include <thread>


int main(int argc, char* argv[])
{
    int port = 6379; //default
    if (argc >= 2)
        port = std::stoi(argv[1]);

    RedisServer server(port);

    // background persistence: dump the database every 300 seconds. (5 min save database)
    std::thread persistenceThread([](){
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(300)); // sleep for 5 minutes
            // Here you would add code to persist/dump the database
        }

    });
    persistenceThread.detach(); // Detach the thread so it runs independently
    server.run(); // Start the server
    

    return 0;
}