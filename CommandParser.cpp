#include "PollHandler.hpp"
#include "CommandParser.hpp"
#include "Utils.hpp"
#include "IRCMessage.hpp"
#include <unistd.h>
#include <iostream>


void CommandParser::parseCommand(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd)
{
    // Handle legacy format for authentication commands
    if (msg.find("PASS ") == 0)
    {
        handlePass(msg, client, server, epoll_fd);
    }
    else if (msg.find("NICK ") == 0)
    {
        handleNick(msg, client, server);
    }
    else if (msg.find("USER ") == 0)
    {
        handleUser(msg, client, server);
    }
    else
    {
        // Parse modern IRC message format
        IRCMessage ircMsg = IRCMessage::parse(msg);
        
        if (ircMsg.command == "PRIVMSG")
        {
            handlePrivmsg(ircMsg, client, server);
        }
        else if (ircMsg.command == "JOIN")
        {
            handleJoin(ircMsg, client, server);
        }
        else if (ircMsg.command == "PART")
        {
            handlePart(ircMsg, client, server);
        }
        else if (ircMsg.command == "NAMES")
        {
            handleNames(ircMsg, client, server);
        }
        else if (ircMsg.command == "LIST")
        {
            handleList(ircMsg, client, server);
        }
        else if (ircMsg.command == "TOPIC")
        {
            handleTopic(ircMsg, client, server);
        }
        else if (ircMsg.command == "QUIT")
        {
            handleQuit(ircMsg, client, server, epoll_fd);
        }
        else
        {
            // Unknown command
            server.sendMessage(client->getFd(), ":server 421 " + client->getNickname() + 
                             " " + ircMsg.command + " :Unknown command\r\n");
        }
    }
}

void CommandParser::sendWelcome(Client* client, ServerSocket& server)
{
    std::string nickname = client->getNickname();
    std::string username = client->getUsername();
    std::string host = "localhost";
    server.sendMessage(client->getFd(), ":server 001 " + nickname + " :Welcome to the IRC Network, " + nickname + "!" + username + "@" + host + "\r\n");
    server.sendMessage(client->getFd(), ":server 002 " + nickname + " :Your host is server, running version 1.0\r\n");
    server.sendMessage(client->getFd(), ":server 003 " + nickname + " :This server was created\r\n");
    server.sendMessage(client->getFd(), ":server 004 " + nickname + " server 1.0 io k\r\n");
}

void CommandParser::handlePass(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd)
{
    std::string pass = Utils::trim(msg.substr(5));
    int fd = client->getFd();
    std::string nickname;

    if (client->getNickname().empty())
    {
        nickname = "*";
    }
    else
    {
        nickname = client->getNickname();
    }
    if (pass.empty())
    {
        server.sendMessage(fd, ":server 464 " + nickname + " :Password incorrect\r\n");
        return;
    }
    if (pass == server.getPassword())
    {
        server.sendMessage(fd, ":Server 001 :Authentication successful\r\n");
        client->setAuthenticated(true);
    }
    else
    {
        server.sendMessage(fd, ":server 464 " + nickname + " :Password incorrect\r\n");
        server.getClients().erase(fd);
        close(fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    }
}

void CommandParser::handleNick(const std::string& msg, Client* client, ServerSocket& server)
{
    std::string nick = Utils::trim(msg.substr(5));
    int fd = client->getFd();
    std::string nickname;

    if (client->getNickname().empty())
    {
        nickname = "*";
    }
    else
    {
        nickname = client->getNickname();
    }
    if (!client->getIsAuthenticated())
    {
        server.sendMessage(fd, ":server 451 " + nickname + " :You have not registered\r\n");
        return;
    }
    if (!Utils::isValidNickname(nick))
    {
        server.sendMessage(fd, ":server 432 " + nickname + " :Invalid nickname\r\n");
        return;
    }
    std::map<int, Client*>::const_iterator it;
    for (it = server.getClients().begin(); it != server.getClients().end(); ++it)
    {
        if (it->second->getNickname() == nick && it->first != client->getFd())
        {
            server.sendMessage(fd, ":server 433 " + nickname + " :Nickname in use\r\n");
            return;
        }
    }
    client->setNickname(nick);
    if (!client->getUsername().empty())
    {
        sendWelcome(client, server);
    }
}

void CommandParser::handleUser(const std::string& msg, Client* client, ServerSocket& server)
{
    int fd = client->getFd();
    std::string nickname;

    if (client->getNickname().empty())
    {
        nickname = "*";
    }
    else
    {
        nickname = client->getNickname();
    }
    if (!client->getIsAuthenticated())
    {
        server.sendMessage(fd, ":server 451 " + nickname + " :You have not registered\r\n");
        return;
    }
    if (!client->getUsername().empty())
    {
        server.sendMessage(fd, ":server 462 " + nickname + " :You may not reregister\r\n");
        return;
    }
    std::string params = Utils::trim(msg.substr(5));
    if (params.empty())
    {
        server.sendMessage(fd, ":server 461 " + nickname + " USER :Not enough parameters\r\n");
        return;
    }
    size_t realname_pos = params.find(" :");
    if (realname_pos == std::string::npos)
    {
        server.sendMessage(fd, ":server 461 " + nickname + " USER :Not enough parameters\r\n");
        return;
    }
    std::string first_part = params.substr(0, realname_pos);
    std::string realname = params.substr(realname_pos + 2);
    std::vector<std::string> tokens = Utils::split(first_part, ' ');
    if (tokens.size() < 3)
    {
        server.sendMessage(fd, ":server 461 " + nickname + " USER :Not enough parameters\r\n");
        return;
    }
    std::string username = Utils::trim(tokens[0]);
    std::string hostname = Utils::trim(tokens[1]); 
    std::string servername = Utils::trim(tokens[2]);
    if (username.empty() || username.length() > 10 || !Utils::isValidUsername(username))
    {
        server.sendMessage(fd, ":server 461 " + nickname + " USER :Invalid username\r\n");
        return;
    }
    if (realname.empty() || realname.length() > 50)
    {
        server.sendMessage(fd, ":server 461 " + nickname + " USER :Invalid realname\r\n");
        return;
    }
    client->setUsername(username);
    client->setRealname(realname);
    if (!client->getNickname().empty())
    {
        sendWelcome(client, server);
    }
}

// PRIVMSG implementation
void CommandParser::handlePrivmsg(const IRCMessage& msg, Client* client, ServerSocket& server)
{
    if (!client->getIsAuthenticated() || client->getNickname().empty())
    {
        server.sendMessage(client->getFd(), ":server 451 * :You have not registered\r\n");
        return;
    }
    
    if (msg.params.empty() || msg.trailing.empty())
    {
        server.sendMessage(client->getFd(), ":server 411 " + client->getNickname() + 
                         " :No recipient given\r\n");
        return;
    }
    
    std::string target = msg.params[0];
    std::string message = msg.trailing;
    
    // Check if target is a channel
    if (target[0] == '#' || target[0] == '&')
    {
        Channel* channel = server.getChannelManager().getChannel(target);
        if (!channel)
        {
            server.sendMessage(client->getFd(), ":server 403 " + client->getNickname() + 
                             " " + target + " :No such channel\r\n");
            return;
        }
        
        if (!channel->isMember(client->getFd()))
        {
            server.sendMessage(client->getFd(), ":server 404 " + client->getNickname() + 
                             " " + target + " :Cannot send to channel\r\n");
            return;
        }
        
        // Broadcast to channel members
        std::string broadcastMsg = ":" + client->getNickname() + "!" + client->getUsername() + 
                                 "@localhost PRIVMSG " + target + " :" + message + "\r\n";
        broadcastToChannel(target, broadcastMsg, server, client->getFd());
    }
    else
    {
        // Private message to user
        Client* targetClient = NULL;
        std::map<int, Client*>& clients = server.getClients();
        for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->second->getNickname() == target)
            {
                targetClient = it->second;
                break;
            }
        }
        
        if (!targetClient)
        {
            server.sendMessage(client->getFd(), ":server 401 " + client->getNickname() + 
                             " " + target + " :No such nick\r\n");
            return;
        }
        
        std::string privateMsg = ":" + client->getNickname() + "!" + client->getUsername() + 
                               "@localhost PRIVMSG " + target + " :" + message + "\r\n";
        server.sendMessage(targetClient->getFd(), privateMsg);
    }
}

// JOIN implementation
void CommandParser::handleJoin(const IRCMessage& msg, Client* client, ServerSocket& server)
{
    if (!client->getIsAuthenticated() || client->getNickname().empty())
    {
        server.sendMessage(client->getFd(), ":server 451 * :You have not registered\r\n");
        return;
    }
    
    if (msg.params.empty())
    {
        server.sendMessage(client->getFd(), ":server 461 " + client->getNickname() + 
                         " JOIN :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = msg.params[0];
    
    if (!isValidChannelName(channelName))
    {
        server.sendMessage(client->getFd(), ":server 403 " + client->getNickname() + 
                         " " + channelName + " :No such channel\r\n");
        return;
    }
    
    // Get or create channel
    Channel* channel = server.getChannelManager().getChannel(channelName);
    if (!channel)
    {
        channel = server.getChannelManager().createChannel(channelName);
    }
    
    // Check if already in channel
    if (channel->isMember(client->getFd()))
    {
        return; // Already in channel, ignore
    }
    
    // Add client to channel
    if (channel->addMember(client->getFd()))
    {
        client->addChannel(channelName);
        
        // Send JOIN message to all channel members
        std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() + 
                            "@localhost JOIN " + channelName + "\r\n";
        broadcastToChannel(channelName, joinMsg, server);
        
        // Send topic if exists
        if (!channel->getTopic().empty())
        {
            server.sendMessage(client->getFd(), ":server 332 " + client->getNickname() + 
                             " " + channelName + " :" + channel->getTopic() + "\r\n");
        }
        
        // Send names list
        std::string membersList = channel->getMembersList(server.getClients());
        server.sendMessage(client->getFd(), ":server 353 " + client->getNickname() + 
                         " = " + channelName + " :" + membersList + "\r\n");
        server.sendMessage(client->getFd(), ":server 366 " + client->getNickname() + 
                         " " + channelName + " :End of /NAMES list\r\n");
    }
}

// PART implementation
void CommandParser::handlePart(const IRCMessage& msg, Client* client, ServerSocket& server)
{
    if (!client->getIsAuthenticated() || client->getNickname().empty())
    {
        server.sendMessage(client->getFd(), ":server 451 * :You have not registered\r\n");
        return;
    }
    
    if (msg.params.empty())
    {
        server.sendMessage(client->getFd(), ":server 461 " + client->getNickname() + 
                         " PART :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = msg.params[0];
    std::string partMessage = msg.trailing.empty() ? client->getNickname() : msg.trailing;
    
    Channel* channel = server.getChannelManager().getChannel(channelName);
    if (!channel || !channel->isMember(client->getFd()))
    {
        server.sendMessage(client->getFd(), ":server 442 " + client->getNickname() + 
                         " " + channelName + " :You're not on that channel\r\n");
        return;
    }
    
    // Send PART message to all channel members
    std::string partMsg = ":" + client->getNickname() + "!" + client->getUsername() + 
                        "@localhost PART " + channelName + " :" + partMessage + "\r\n";
    broadcastToChannel(channelName, partMsg, server);
    
    // Remove from channel
    channel->removeMember(client->getFd());
    client->removeChannel(channelName);
    
    // Remove empty channels
    if (channel->isEmpty())
    {
        server.getChannelManager().removeChannel(channelName);
    }
}

// NAMES implementation
void CommandParser::handleNames(const IRCMessage& msg, Client* client, ServerSocket& server)
{
    if (!client->getIsAuthenticated() || client->getNickname().empty())
    {
        server.sendMessage(client->getFd(), ":server 451 * :You have not registered\r\n");
        return;
    }
    
    if (msg.params.empty())
    {
        // List all channels user is in
        const std::set<std::string>& userChannels = client->getChannels();
        for (std::set<std::string>::const_iterator it = userChannels.begin(); 
             it != userChannels.end(); ++it)
        {
            Channel* channel = server.getChannelManager().getChannel(*it);
            if (channel)
            {
                std::string membersList = channel->getMembersList(server.getClients());
                server.sendMessage(client->getFd(), ":server 353 " + client->getNickname() + 
                                 " = " + *it + " :" + membersList + "\r\n");
            }
        }
    }
    else
    {
        std::string channelName = msg.params[0];
        Channel* channel = server.getChannelManager().getChannel(channelName);
        if (channel)
        {
            std::string membersList = channel->getMembersList(server.getClients());
            server.sendMessage(client->getFd(), ":server 353 " + client->getNickname() + 
                             " = " + channelName + " :" + membersList + "\r\n");
        }
    }
    
    server.sendMessage(client->getFd(), ":server 366 " + client->getNickname() + 
                     " * :End of /NAMES list\r\n");
}

// LIST implementation
void CommandParser::handleList(const IRCMessage& /* msg */, Client* client, ServerSocket& server)
{
    if (!client->getIsAuthenticated() || client->getNickname().empty())
    {
        server.sendMessage(client->getFd(), ":server 451 * :You have not registered\r\n");
        return;
    }
    
    server.sendMessage(client->getFd(), ":server 321 " + client->getNickname() + 
                     " Channel :Users  Name\r\n");
    
    std::vector<std::string> channels = server.getChannelManager().getChannelList();
    for (size_t i = 0; i < channels.size(); ++i)
    {
        Channel* channel = server.getChannelManager().getChannel(channels[i]);
        if (channel)
        {
            server.sendMessage(client->getFd(), ":server 322 " + client->getNickname() + 
                             " " + channel->getName() + " " + Utils::intToString(channel->getMemberCount()) + 
                             " :" + channel->getTopic() + "\r\n");
        }
    }
    
    server.sendMessage(client->getFd(), ":server 323 " + client->getNickname() + 
                     " :End of /LIST\r\n");
}

// TOPIC implementation
void CommandParser::handleTopic(const IRCMessage& msg, Client* client, ServerSocket& server)
{
    if (!client->getIsAuthenticated() || client->getNickname().empty())
    {
        server.sendMessage(client->getFd(), ":server 451 * :You have not registered\r\n");
        return;
    }
    
    if (msg.params.empty())
    {
        server.sendMessage(client->getFd(), ":server 461 " + client->getNickname() + 
                         " TOPIC :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = msg.params[0];
    Channel* channel = server.getChannelManager().getChannel(channelName);
    
    if (!channel)
    {
        server.sendMessage(client->getFd(), ":server 403 " + client->getNickname() + 
                         " " + channelName + " :No such channel\r\n");
        return;
    }
    
    if (!channel->isMember(client->getFd()))
    {
        server.sendMessage(client->getFd(), ":server 442 " + client->getNickname() + 
                         " " + channelName + " :You're not on that channel\r\n");
        return;
    }
    
    if (msg.trailing.empty())
    {
        // Query topic
        if (channel->getTopic().empty())
        {
            server.sendMessage(client->getFd(), ":server 331 " + client->getNickname() + 
                             " " + channelName + " :No topic is set\r\n");
        }
        else
        {
            server.sendMessage(client->getFd(), ":server 332 " + client->getNickname() + 
                             " " + channelName + " :" + channel->getTopic() + "\r\n");
        }
    }
    else
    {
        // Set topic
        if (channel->isTopicRestricted() && !channel->isOperator(client->getFd()))
        {
            server.sendMessage(client->getFd(), ":server 482 " + client->getNickname() + 
                             " " + channelName + " :You're not channel operator\r\n");
            return;
        }
        
        channel->setTopic(msg.trailing);
        std::string topicMsg = ":" + client->getNickname() + "!" + client->getUsername() + 
                             "@localhost TOPIC " + channelName + " :" + msg.trailing + "\r\n";
        broadcastToChannel(channelName, topicMsg, server);
    }
}

// QUIT implementation
void CommandParser::handleQuit(const IRCMessage& msg, Client* client, ServerSocket& server, int epoll_fd)
{
    std::string quitMessage = msg.trailing.empty() ? "Client Quit" : msg.trailing;
    
    // Send quit message to all channels the user is in
    const std::set<std::string>& userChannels = client->getChannels();
    for (std::set<std::string>::const_iterator it = userChannels.begin(); 
         it != userChannels.end(); ++it)
    {
        std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername() + 
                            "@localhost QUIT :" + quitMessage + "\r\n";
        broadcastToChannel(*it, quitMsg, server, client->getFd());
    }
    
    // Remove from all channels
    server.getChannelManager().removeClientFromAllChannels(client->getFd());
    
    // Close connection
    server.sendMessage(client->getFd(), ":server QUIT :" + quitMessage + "\r\n");
    close(client->getFd());
    server.getClients().erase(client->getFd());
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->getFd(), NULL);
}

// Helper functions
bool CommandParser::isValidChannelName(const std::string& name)
{
    if (name.empty() || name.length() > 50)
        return false;
    
    return (name[0] == '#' || name[0] == '&');
}

void CommandParser::broadcastToChannel(const std::string& channelName, const std::string& message, 
                                     ServerSocket& server, int excludeFd)
{
    Channel* channel = server.getChannelManager().getChannel(channelName);
    if (!channel)
        return;
    
    const std::set<int>& members = channel->getMembers();
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (*it != excludeFd)
        {
            server.sendMessage(*it, message);
        }
    }
}