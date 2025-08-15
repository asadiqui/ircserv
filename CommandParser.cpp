#include "PollHandler.hpp"
#include "CommandParser.hpp"
#include "Utils.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

void CommandParser::parseCommand(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd, Bot& bot)
{
    if (msg.find("PASS ") == 0)
    {
        std::cout << "PASS command received" << std::endl;
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
    else if (msg.find("PRIVMSG ") == 0)
    {
        handlePrivmsg(msg, client, server, bot);
    }
    else if (msg.find("NOTICE ") == 0)
    {
        handleNotice(msg, client, server);
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

void CommandParser::handleDccSend(std::string target ,const std::string& msg, Client* client, ServerSocket& server)
{
    // command is : "\x01DCC SEND <filename> <size> <port>\x01"
    std::string dccmsg = ":" + client->getUsername() + "!" + client->getNickname() + "@localhost PRIVMSG " + target + " :\001" + msg + "\001\r\n";
    send(client->getFd(), dccmsg.c_str(), dccmsg.length(), 0);
    std::istringstream len(msg);
    std::string skip, command;
    len >> skip >> command;
    if (command == "SEND")
    {
        std::string filename;
        len >> filename;
        std::string notice = ":localhost NOTICE " + target + " :DCC SEND request for file '" + filename + "' received.\r\n";
        server.sendMessage(client->getFd(), notice);
    }
}

bool isdccessage(const std::string& message)
{
    return (message.length() >= 2 && message[0] == '\x01' && message[message.length() - 1] == '\x01');
}

void CommandParser::handlePrivmsg(const std::string& msg, Client* client, ServerSocket& server, Bot& bot)
{
    std::string params = msg.substr(8);
    size_t le = params.find(" :");
    std::string target = params.substr(0, le); // Extract target
    std::cout << "Target: " << target << std::endl;
    std::string command = params.substr(le + 2);
    if (target == bot.getNickname())
    {
        if (command == "HELLO")
        {
            std::string response = ":server PRIVMSG " + client->getNickname() + " :Hello, I am " + bot.getNickname() + ". How can I assist you?\r\n";
            server.sendMessage(client->getFd(), response);
        }
        else if (command == "HELP")
        {
            std::string helpMessage = ":server PRIVMSG " + client->getNickname() + " :Available commands: NICK, USER, PRIVMSG, NOTICE, DCC SEND\r\n";
            server.sendMessage(client->getFd(), helpMessage);
        }
        else if (command == "VERSION")
        {
            std::string versionMessage = ":server PRIVMSG " + client->getNickname() + " :IRC Bot Version 1.0\r\n";
            server.sendMessage(client->getFd(), versionMessage);
        }
        else if (command == "PING")
        {
            std::string pingMessage = ":server PONG " + client->getNickname() + "\r\n";
            server.sendMessage(client->getFd(), pingMessage);
        }
        else if (command == "QUIT")
        {
            std::string quitMessage = ":server PRIVMSG " + client->getNickname() + " :Goodbye!\r\n";
            server.sendMessage(client->getFd(), quitMessage);
        }

        return;
    }
    std::string massage;
    if (le != std::string::npos)
    {
        massage = msg.substr(le + 2); 
    }

    if (isdccessage(massage))
    {
        handleDccSend(target,msg, client, server);
        return;
    }

}

void CommandParser::handleNotice(const std::string& msg, Client* client, ServerSocket& server)
{
    std::string parmas = msg.substr(7); // Skip "NOTICE "
    std::vector<std::string> tokens = Utils::split(parmas, ' '); // Split by space
    std::string name = tokens[0]; // Get the name
    Client* targetClient = NULL; // Initialize targetClient to NULL
    std::map<int, Client*>::const_iterator it;

    for (it = server.getClients().begin(); it != server.getClients().end(); ++it)
    {
        if (it->second->getNickname() == name)
        {
            targetClient = it->second; // Find the target client
            std::cout << "Found target client: " << targetClient->getNickname() << std::endl;
            break;
        }
    }
    if (targetClient == NULL)
    {
        server.sendMessage(client->getFd(), ":server 401 " + client->getNickname() + " " + name + " :No such nick/channel\r\n");
        return;
    }
    std::string message;
    for (size_t i = 1; i < tokens.size(); ++i)
    {
        message += tokens[i] + " "; // Concatenate the message
    }    int check = 0;
    if (tokens[0][0] == '#')
        check = 1; // Check if the first token is a channel
    if (check == 0)
    {
        std::string noticeMsg = ":" + client->getNickname() + " NOTICE " + name + " :" + message + "\r\n";
        server.sendMessage(targetClient->getFd(), noticeMsg);
        return;
    }

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
    if (client->getIsAuthenticated() && !client->getNickname().empty() && !client->getUsername().empty())
    {
        server.sendMessage(fd, ":server 462 " + nickname + " :You may not reregister\r\n");
        return;
    }
    std::string params = msg.substr(5);
    std::vector<std::string> tokens = Utils::split(params, ' ');
    if (tokens.size() < 4)
    {
        server.sendMessage(fd, ":server 461 " + nickname + " :Need more parameters\r\n");
        return;
    }
    std::string username = tokens[0];
    std::string hostname = tokens[1];
    std::string servername = tokens[2];
    std::string realname = tokens[3];
    if (hostname.empty() || servername.empty() || hostname.find(' ') != std::string::npos)
    {
        server.sendMessage(fd, ":server 461 " + nickname + " :Need more parameters\r\n");
        return;
    }
    if (username.empty() || realname.empty())
    {
        server.sendMessage(fd, ":server 461 " + nickname + " :Need more parameters\r\n");
        return;
    }
    client->setUsername(username);
    client->setRealname(realname);
    if (!client->getNickname().empty())
    {
        sendWelcome(client, server);
    }
}