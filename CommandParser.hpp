#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include "Client.hpp"
#include "ServerSocket.hpp"

class CommandParser 
{
public:
    static void parseCommand(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd);
private:
    static void handlePass(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd);
    static void handleNick(const std::string& msg, Client* client, ServerSocket& server);
    static void handleUser(const std::string& msg, Client* client, ServerSocket& server);
};

#endif