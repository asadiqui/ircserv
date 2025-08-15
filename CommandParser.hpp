#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include "Client.hpp"
#include "ServerSocket.hpp"
#include "IRCMessage.hpp"

class CommandParser 
{
public:
    static void parseCommand(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd);
private:
    static void handlePass(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd);
    static void handleNick(const std::string& msg, Client* client, ServerSocket& server);
    static void handleUser(const std::string& msg, Client* client, ServerSocket& server);
    static void sendWelcome(Client* client, ServerSocket& server);
    
    // New IRC commands
    static void handlePrivmsg(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleJoin(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handlePart(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleNames(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleList(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleTopic(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleQuit(const IRCMessage& msg, Client* client, ServerSocket& server, int epoll_fd);
    
    // Helper functions
    static bool isValidChannelName(const std::string& name);
    static void broadcastToChannel(const std::string& channelName, const std::string& message, 
                                 ServerSocket& server, int excludeFd = -1);
};

#endif