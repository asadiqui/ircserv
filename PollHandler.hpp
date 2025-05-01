#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP

#include <poll.h>
#include <map>
#include <string>
#include "ServerSocket.hpp"
#include <vector>

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
};

#endif