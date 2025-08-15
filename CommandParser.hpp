#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include "Client.hpp"
#include "ServerSocket.hpp"
#include "bot.hpp"

class CommandParser 
{
public:
    static void parseCommand(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd, Bot& bot);
private:
    static void handlePass(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd);
    static void handleNick(const std::string& msg, Client* client, ServerSocket& server);
    static void handleUser(const std::string& msg, Client* client, ServerSocket& server);
    static void sendWelcome(Client* client, ServerSocket& server);
    static void handlePrivmsg(const std::string& msg, Client* client, ServerSocket& server , Bot& bot);
    static void handleNotice(const std::string& msg, Client* client, ServerSocket& server);
    static void handleDccSend(std::string target ,const std::string& msg, Client* client, ServerSocket& server);
    static void handleDccAccept(const std::string& msg, Client* client, ServerSocket& server);
    
};

#endif