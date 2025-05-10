#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP

#include <map>
#include <string>
#include <vector>
#include <sys/epoll.h>
#include "ServerSocket.hpp"

class PollHandler 
{
private:
    ServerSocket& server;
    int epoll_fd;
    std::map<int, std::string> clientBuffers;
public:
    PollHandler(ServerSocket& server);
    ~PollHandler();
    void run();
    void processClientMessage(int fd);
    void closeClient(int fd);
    void addClientToEpoll(int fd);
};

#endif