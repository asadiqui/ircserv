#include "PollHandler.hpp"
#include "ServerSocket.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "CommandParser.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>

PollHandler::PollHandler(ServerSocket& server) : server(server)
{
    epoll_fd = epoll_create(1);
    if (epoll_fd == -1)
    {
        throw std::runtime_error("epoll_create failed: " + std::string(strerror(errno)));
    }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server.getSocketFd();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server.getSocketFd(), &ev) == -1)
    {
        close(epoll_fd);
        throw std::runtime_error("epoll_ctl failed for server socket: " + std::string(strerror(errno)));
    }
}

PollHandler::~PollHandler() 
{
    close(epoll_fd);
}

void PollHandler::processClientMessage(int fd)
{
    char buffer[Config::BUFFER_SIZE];
    while (true)
    {
        int r_bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (r_bytes <= 0) 
        {
            if (r_bytes == 0 || (r_bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) 
            {
                closeClient(fd);
            }
            break;
        }
        buffer[r_bytes] = '\0';
        clientBuffers[fd] += buffer;
        std::string& clientBuffer = clientBuffers[fd];
        size_t pos;
        while ((pos = clientBuffer.find("\r\n")) != std::string::npos)
        {
            std::string msg = clientBuffer.substr(0, pos);
            clientBuffer.erase(0, pos + 2);
            Logger::info("Received from FD[" + Utils::intToString(fd) + "]: " + msg);
            std::map<int, Client*>::iterator it = server.getClients().find(fd);
            if (it != server.getClients().end())
            {
                CommandParser::parseCommand(msg, it->second, server, epoll_fd);
            }
        }
    }
}

void PollHandler::closeClient(int fd)
{
    server.sendMessage(fd, ":Server QUIT :Connection closed\r\n");
    close(fd);
    server.getClients().erase(fd);
    clientBuffers.erase(fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    Logger::info("Client FD[" + Utils::intToString(fd) + "] closed");
}

void PollHandler::addClientToEpoll(int fd)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) 
    {
        Logger::error("epoll_ctl failed for FD " + Utils::intToString(fd) + ": " + strerror(errno));
        close(fd);
    }
}

void PollHandler::run() 
{
    std::vector<epoll_event> events(Config::MAX_EVENTS);
    while (true) 
    {
        int nfds = epoll_wait(epoll_fd, events.data(), Config::MAX_EVENTS, -1);
        if (nfds == -1) 
        {
            if (errno == EINTR) 
                continue;
            Logger::error("epoll_wait failed: " + std::string(strerror(errno)));
            continue;
        }
        for (int i = 0; i < nfds; i++) 
        {
            int fd = events[i].data.fd;
            if (fd == server.getSocketFd()) 
            {
                server.newClient(*this);
            }
            else if (events[i].events & EPOLLIN)
            {
                processClientMessage(fd);
            }
            else if (events[i].events & (EPOLLHUP | EPOLLERR))
            {
                closeClient(fd);
            }
        }
    }
}