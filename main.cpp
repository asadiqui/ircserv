#include "ServerSocket.hpp"
#include "PollHandler.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

void signalHandler(int code) 
{
    std::cout << "Signal " << code << " caught!" << std::endl;
    exit(0);
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    int port;
    try 
    {
        port = std::atoi(argv[1]);
        if (port < 0 || port > 65535) 
        {
            std::cout << "Port must be between 0 and 65535!" << std::endl;
            return 1;
        }
        for (int i = 0; argv[1][i]; i++)
        {
            if (argv[1][i] < '0' || argv[1][i] > '9')
            {
                std::cout << "Port must be a number!" << std::endl;
                return 1;
            }
        }
    }
    catch (...) 
    {
        std::cout << "Invalid port!" << std::endl;
        return 1;
    }
    std::string password = argv[2];
    signal(SIGINT, signalHandler);
    try 
    {
        ServerSocket server(port, password);
        PollHandler poller(server);
        poller.run(); // Runs indefinitely
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}