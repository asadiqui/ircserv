#include "ServerSocket.hpp"
#include <stdexcept>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <netdb.h>
#include <poll.h>
#include <cstdio>
#include <sys/epoll.h>

ServerSocket::ServerSocket(int port, std::string password) : port(port), password(password), socketFd(-1)
{
    struct addrinfo hints, *serverInfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char portStr[10];
    sprintf(portStr, "%d", port);
    int status = getaddrinfo(NULL, portStr, &hints, &serverInfo);
    if (status != 0)
    {
        std::string errorMsg = "getaddrinfo error: ";
        errorMsg += gai_strerror(status);
        throw std::runtime_error(errorMsg);
    }
    socketFd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (socketFd == -1)
        throw std::runtime_error("Server socket could not be initialized!");
    if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Failed to set server socket non-blocking!");
    int yes = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        throw std::runtime_error("Failed to set socket options!");
    if (bind(socketFd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1)
        throw std::runtime_error("Server socket could not be bound!");
    if (listen(socketFd, 10) == -1)
        throw std::runtime_error("Server socket could not be listened!");
    freeaddrinfo(serverInfo);
    std::cout << "Server socket initialized on port " << port << std::endl;
}

void ServerSocket::newClient(int epoll_fd)
{
    struct sockaddr_in newClientAddr;
    socklen_t addrLen = sizeof(newClientAddr);
    int newClient = accept(socketFd, reinterpret_cast<sockaddr*>(&newClientAddr), &addrLen);
    if (newClient == -1)
    {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }
    if (fcntl(newClient, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "fcntl failed client FD " << newClient << ": " << strerror(errno) << std::endl;
        close(newClient);
        return;
    }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = newClient;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newClient, &ev) == -1)
    {
        std::cerr << "epoll_ctl failed for client FD " << newClient << ": " << strerror(errno) << std::endl;
        close(newClient);
        return;
    }
    clients[newClient] = new Client(newClient);
    sender(newClient, "SERVER You must authenticate!\r\n");
    sender(newClient, "SERVER For help use HELP command!\r\n");
    std::cout << "Client new open: FD " << newClient << std::endl;
}

void ServerSocket::sender(int fd, const std::string& msg)
{
    std::cout << "Send ----> Fd[" << fd << "]: " << msg << std::endl;
    if (send(fd, msg.c_str(), msg.size(), 0) == -1)
    {
        std::cerr << "Send failed for FD " << fd << ": " << strerror(errno) << std::endl;
    }
}

ServerSocket::~ServerSocket()
{
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        close(it->first);
        delete it->second;
    }
    close(socketFd);
    std::cout << "Server socket closed" << std::endl;
}
