#ifndef POLLHANDLER_HPP
#define POLLHANDLER_HPP

#include <map>
#include <string>
#include <poll.h>
#include <cerrno>

class Client;
class ServerSocket;

class PollHandler
{
private:
    pollfd fds[10];
    int max_fds;
    std::map<int, std::string> clientBuffers;
    ServerSocket& server;

public:
    PollHandler(ServerSocket& server);
    void run();
};

#endif