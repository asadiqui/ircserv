#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include "Client.hpp"
#include "ServerSocket.hpp"
#include "IRCMessage.hpp"
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
    static void handlePing(const std::string& msg, Client* client, ServerSocket& server);
    // New IRC commands
    static void handlePrivmsg(const IRCMessage& msg, Client* client, ServerSocket& server, Bot& bot);
    static void handleJoin(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handlePart(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleNames(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleList(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleTopic(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleQuit(const IRCMessage& msg, Client* client, ServerSocket& server, int epoll_fd);
    static void handleKick(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleDccSend(Client* Client2, std::string target ,const std::string& msg, Client* client, ServerSocket& server);
    static void handleInvite(const IRCMessage& msg, Client* client, ServerSocket& server);
    static void handleMode(const IRCMessage& msg, Client* client, ServerSocket& server);

    
    // Helper functions
    static bool isValidChannelName(const std::string& name);
    static void broadcastToChannel(const std::string& channelName, const std::string& message, 
                                 ServerSocket& server, int excludeFd = -1);
};

#endif