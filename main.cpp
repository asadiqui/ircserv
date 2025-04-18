#include "ServerSocket.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <stdexcept>


int main(int argc, char** argv)
{
    int port;
    if (argc != 3)
    {
        std::cout << "./ircserv <port> <password>" << std::endl;
        return 1;
    }
    try 
    {
        port = std::atoi(argv[1]);
        if (port < 1024 || port > 65535) 
        {
            std::cout << "Port must be between 1024 and 65535!" << std::endl;
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
    try 
    {
        std::cout << "Starting IRC server on port " << port << "..." << std::endl;
        ServerSocket server(port, password);
        server.run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl; 
        return 1;
    }
    return 0;
}