#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP

#include <map>
#include <string>
#include <vector>
#include <sys/epoll.h>
#include "ServerSocket.hpp"
#include "bot.hpp"

class Bot;

class PollHandler 
{
private:
    ServerSocket& server;
    int epoll_fd;
    std::map<int, std::string> clientBuffers;
public:
    PollHandler(ServerSocket& server);
    ~PollHandler();
    void run(Bot& bot);
    void processClientMessage(int fd, Bot& bot);
    void closeClient(int fd);
    void addClientToEpoll(int fd);
};

#endif