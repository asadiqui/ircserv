#include "ServerSocket.hpp"
#include "Logger.hpp"
#include "PollHandler.hpp"
#include "Utils.hpp"
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <sys/epoll.h>
#include <cerrno>

ServerSocket::ServerSocket(int port, std::string password) : port(port), password(password), socketFd(-1)
{
    struct addrinfo hints, *serverInfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::string portStr = Utils::intToString(port);
    int status = getaddrinfo(NULL, portStr.c_str(), &hints, &serverInfo);
    if (status != 0)
        throw std::runtime_error("getaddrinfo error: " + std::string(gai_strerror(status)));
    socketFd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (socketFd == -1) 
    {
        freeaddrinfo(serverInfo);
        throw std::runtime_error("Server socket could not be initialized!");
    }
    if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) 
    {
        freeaddrinfo(serverInfo);
        throw std::runtime_error("Failed to set server socket non-blocking!");
    }
    int yes = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
    {
        freeaddrinfo(serverInfo);
        throw std::runtime_error("Failed to set socket options!");
    }
    if (bind(socketFd, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1)
    {
        freeaddrinfo(serverInfo);
        throw std::runtime_error("Server socket could not be bound!");
    }
    if (listen(socketFd, 10) == -1) 
    {
        freeaddrinfo(serverInfo);
        throw std::runtime_error("Server socket could not be listened!");
    }
    freeaddrinfo(serverInfo);
    Logger::info("Server socket initialized on port " + Utils::intToString(getPort()));
}

void ServerSocket::newClient(PollHandler& poller)
{
    struct sockaddr_in newClientAddr;
    socklen_t addrLen = sizeof(newClientAddr);
    int newClient = accept(socketFd, reinterpret_cast<sockaddr*>(&newClientAddr), &addrLen);
    if (newClient == -1)
    {
        Logger::error("Accept failed: " + std::string(strerror(errno)));
        return;
    }
    if (fcntl(newClient, F_SETFL, O_NONBLOCK) == -1) 
    {
        Logger::error("fcntl failed for FD " + Utils::intToString(newClient) + ": " + strerror(errno));
        close(newClient);
        return;
    }
    clientManager.addClient(newClient, new Client(newClient));
    poller.addClientToEpoll(newClient);
    sendMessage(newClient, "SERVER You must authenticate!\r\n");
    sendMessage(newClient, "SERVER For help use HELP command!\r\n");
    Logger::info("Client FD[" + Utils::intToString(newClient) + "] connected");
}

bool ServerSocket::sendMessage(int fd, const std::string& msg)
{
    Logger::info("Send ----> FD[" + Utils::intToString(fd) + "]: " + msg);
    if (send(fd, msg.c_str(), msg.size(), 0) == -1)
    {
        Logger::error("Send failed for FD " + Utils::intToString(fd) + ": " + strerror(errno));
        return false;
    }
    return true;
}

ServerSocket::~ServerSocket()
{
    for (std::map<int, Client*>::iterator it = clientManager.getClients().begin(); \
            it != clientManager.getClients().end(); ++it) 
    {
        close(it->first);
        delete it->second;
    }
    close(socketFd);
    Logger::info("Server socket closed");
}