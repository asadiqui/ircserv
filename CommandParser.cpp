#include "PollHandler.hpp"
#include "CommandParser.hpp"
#include "Utils.hpp"
#include <unistd.h>
#include <iostream>


void CommandParser::parseCommand(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd)
{
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