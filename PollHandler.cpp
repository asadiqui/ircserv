#include "PollHandler.hpp"
#include "ServerSocket.hpp"
#include "Client.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define BUFFER_SIZE 1024
#define MAX_EVENTS 100

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

bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;
    for (unsigned int i = 0; i < nick.length(); i++)
    {
        char c = nick[i];
        if (!(c >= 'A' && c <= 'Z') && !(c >= 'a' && c <= 'z') &&
            !(c >= '0' && c <= '9') && c != '-' && c != '_')
            return false;
    }
    return true;
}

void PollHandler::run()
{
    struct epoll_event events[MAX_EVENTS];
    while (true)
    {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            continue;
        }
        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;
            if (fd == server.getSocketFd())
            {
                server.newClient(epoll_fd);
            }
            else if (events[i].events & EPOLLIN)
            {
                char buff[BUFFER_SIZE];
                while (true)
                {
                    int r_bytes = recv(fd, buff, sizeof(buff) - 1, 0);
                    if (r_bytes <= 0)
                    {
                        if (r_bytes == 0 || (r_bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
                        {
                            server.sender(fd, ":Server QUIT :Connection closed\r\n");
                            close(fd);
                            server.getClients().erase(fd);
                            clientBuffers.erase(fd);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        }
                        break;
                    }
                    buff[r_bytes] = '\0';
                    clientBuffers[fd] += buff;
                    std::string& buffer = clientBuffers[fd];
                    size_t pos;
                    while ((pos = buffer.find("\r\n")) != std::string::npos)
                    {
                        std::string msg = buffer.substr(0, pos);
                        buffer.erase(0, pos + 2);
                        std::cout << "Received from FD[" << fd << "]: " << msg << std::endl;
                        std::map<int, Client*>::iterator it = server.getClients().find(fd);
                        if (it == server.getClients().end())
                            continue;
                        Client* client = it->second;
                        if (!client->getIsAuthenticated())
                        {
                            if (msg.find("PASS ") == 0)
                            {
                                std::string pass = msg.substr(5);
                                std::string::size_type start = 0;
                                while (start < pass.length() && (pass[start] == ' ' || pass[start] == '\t'))
                                    start++;
                                pass = pass.substr(start);
                                std::string::size_type end = pass.length();
                                while (end > 0 && (pass[end - 1] == ' ' || pass[end - 1] == '\t'))
                                    end--;
                                pass = pass.substr(0, end);
                                if (pass.empty())
                                {
                                    server.sender(fd, ":Server 464 :Password incorrect\r\n\r\n");
                                    close(fd);
                                    server.getClients().erase(fd);
                                    clientBuffers.erase(fd);
                                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                                    continue;
                                }
                                if (pass == server.getPassword())
                                {
                                    client->setAuthenticated(true);
                                    server.sender(fd, ":Server 001 :Authentication successful\r\n");
                                }
                                else
                                {
                                    server.sender(fd, ":Server 464 :Password incorrect\r\n");
                                    close(fd);
                                    server.getClients().erase(fd);
                                    clientBuffers.erase(fd);
                                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                                }
                            }
                        }
                        if (msg.find("NICK ") == 0)
                        {
                            std::string nick = msg.substr(5);
                            if (!isValidNickname(nick))
                            {
                                server.sender(fd, ":Server 432 :Invalid nickname\r\n");
                                continue;
                            }
                            bool nickInUse = false;
                            for (std::map<int, Client*>::iterator nickIt = server.getClients().begin();
                                 nickIt != server.getClients().end(); ++nickIt)
                            {
                                if (nickIt->second->getNickname() == nick && nickIt->first != fd)
                                {
                                    nickInUse = true;
                                    break;
                                }
                            }
                            if (nickInUse)
                            {
                                server.sender(fd, ":Server 433 :Nickname in use\r\n");
                                continue;
                            }
                            client->setNickname(nick);
                            server.sender(fd, ":Server 001 :Nickname set\r\n");
                        }
                        if (msg.find("USER ") == 0)
                        {
                            if (client->getIsAuthenticated() && !client->getNickname().empty() && !client->getUsername().empty())
                            {
                                server.sender(fd, ":Server 462 :You may not reregister\r\n");
                                continue;
                            }
                            std::string params = msg.substr(5);
                            std::string tokens[4];
                            int tokenCount = 0;
                            std::string::size_type start = 0, end;
                            while (tokenCount < 4 && (end = params.find(' ', start)) != std::string::npos)
                            {
                                tokens[tokenCount] = params.substr(start, end - start);
                                start = end + 1;
                                tokenCount++;
                            }
                            if (tokenCount < 3)
                            {
                                server.sender(fd, ":Server 461 :Need more parameters\r\n");
                                continue;
                            }
                            tokens[tokenCount] = params.substr(start);
                            tokenCount++;
                            std::string username = tokens[0];
                            std::string hostname = tokens[1];
                            std::string servername = tokens[2];
                            std::string realname = tokens[3];
                            if (username.empty() || realname.empty())
                            {
                                server.sender(fd, ":Server 461 :Need more parameters\r\n");
                                continue;
                            }
                            client->setUsername(username);
                            client->setRealname(realname);
                            server.sender(fd, ":Server 001 :User set\r\n");
                        }
                    }
                }
            }
            if (events[i].events & (EPOLLHUP | EPOLLERR))
            {
                server.sender(fd, ":Server QUIT :Connection error\r\n");
                close(fd);
                server.getClients().erase(fd);
                clientBuffers.erase(fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            }
        }
    }
}