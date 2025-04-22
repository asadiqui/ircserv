#include "PollHandler.hpp"
#include "ServerSocket.hpp"
#include "Client.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>

#define BUFFER_SIZE 1024

PollHandler::PollHandler(ServerSocket& server) : server(server), max_fds(10)
{
    for (int i = 0; i < max_fds; i++)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = server.getSocketFd();
    fds[0].events = POLLIN;
}

void PollHandler::run()
{
    while (true) 
    {
        if (poll(fds, max_fds, -1) == -1)
        {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            continue;
        }
        if (fds[0].revents & POLLIN)
        {
            server.newClient(fds, max_fds);
        }
        for (int i = 1; i < max_fds; i++)
        {
            if (fds[i].fd != -1)
            {
                if (fds[i].revents & POLLIN)
                {
                    char buff[BUFFER_SIZE];
                    int r_bytes = recv(fds[i].fd, buff, sizeof(buff) - 1, 0);
                    if (r_bytes <= 0)
                    {
                        server.sender(fds[i].fd, ":Server QUIT :Connection closed\r\n");
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        continue;
                    }
                    buff[r_bytes] = '\0';
                    clientBuffers[fds[i].fd] += buff;
                    std::string& buffer = clientBuffers[fds[i].fd];
                    size_t pos;
                    while ((pos = buffer.find("\r\n")) != std::string::npos)
                    {
                        std::string msg = buffer.substr(0, pos);
                        buffer.erase(0, pos + 2);
                        std::cout << "Received from FD[" << fds[i].fd << "]: " << msg << std::endl;
                        server.sender(fds[i].fd, "SERVER Echo: " + msg + "\r\n");
                    }
                }
                if (fds[i].revents & (POLLHUP | POLLERR))
                {
                    server.sender(fds[i].fd, ":Server QUIT :Connection error\r\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    clientBuffers.erase(fds[i].fd);
                }
                fds[i].revents = 0;
            }
        }
    }
}