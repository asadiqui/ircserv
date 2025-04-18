#include "ServerSocket.hpp"
#include <stdexcept>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "Client.hpp"
#include <cerrno>

ServerSocket::ServerSocket(int port, std::string password) : port(port), password(password), socketFd(-1) 
{
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    socketFd = socket(sockAddr.sin_family, SOCK_STREAM, 0);
    if (socketFd == -1)
        throw std::runtime_error("Server socket could not initialized!");
    if (bind(socketFd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) == -1)
        throw std::runtime_error("Server socket could not binded!");
    if (listen(socketFd, 10) == -1)
        throw std::runtime_error("Server socket could not be listened!");
    std::cout << "Server socket initialized on port " << port << std::endl;
}

void ServerSocket::sender(int fd, const std::string& msg)
{
    std::cout << "Send ----> Fd[" << fd << "]: " << msg;
    if (send(fd, msg.c_str(), msg.size(), 0) == -1)
    {
        std::cerr << "Send failed for FD " << fd << ": " << strerror(errno) << std::endl;
    }
}

void ServerSocket::run()
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Accept one client
    std::cout << "Server running, waiting for a connection..." << std::endl;
    int clientFd = accept(socketFd, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
    if (clientFd == -1)
    {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }
    std::cout << "Client connected: FD " << clientFd << std::endl;
    currentClient = new Client(clientFd);
    std::string welcome_msg = "Welcome to our IRC server\r\n";
    sender(clientFd, welcome_msg);
    // Receive message from the client
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) 
    {
        std::cerr << "Failed to receive message from client or client disconnected" << std::endl;
    }
    else
    {
        std::string message(buffer);
        std::cout << "Message with length " << bytes_received << " was received from client: " << message << std::endl;
    }
    close(clientFd);
    std::cout << "Client connection closed, server shutting down..." << std::endl;
}

ServerSocket::~ServerSocket()
{
    if (currentClient)
    {
        close(currentClient->getFd());
    }
    close(socketFd);
    std::cout << "Server socket closed" << std::endl;
}