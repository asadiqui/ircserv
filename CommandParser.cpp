#include "PollHandler.hpp"
#include "CommandParser.hpp"
#include "Utils.hpp"
#include <unistd.h>

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

void CommandParser::handlePass(const std::string& msg, Client* client, ServerSocket& server, int epoll_fd) 
{
    std::string pass = Utils::trim(msg.substr(5));
    int fd = client->getFd();
    if (pass.empty()) 
    {
        server.sendMessage(fd, ":Server 464 :Password incorrect\r\n");
        server.getClients().erase(fd);
        close(fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        return;
    }
    if (pass == server.getPassword())
    {
        client->setAuthenticated(true);
        server.sendMessage(fd, ":Server 001 :Authentication successful\r\n");
    } 
    else
    {
        server.sendMessage(fd, ":Server 464 :Password incorrect\r\n");
        server.getClients().erase(fd);
        close(fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    }
}

void CommandParser::handleNick(const std::string& msg, Client* client, ServerSocket& server)
{
    std::string nick = Utils::trim(msg.substr(5));

    if (!Utils::isValidNickname(nick)) 
    {
        server.sendMessage(client->getFd(), ":Server 432 :Invalid nickname\r\n");
        return;
    }
    std::map<int, Client*>::const_iterator it;
    for (it = server.getClients().begin(); it != server.getClients().end(); ++it) 
    {
        if (it->second->getNickname() == nick && it->first != client->getFd())
        {
            server.sendMessage(client->getFd(), ":Server 433 :Nickname in use\r\n");
            return;
        }
    }
    client->setNickname(nick);
    server.sendMessage(client->getFd(), ":Server 001 :Nickname set\r\n");
}

void CommandParser::handleUser(const std::string& msg, Client* client, ServerSocket& server)
{
    if (client->getIsAuthenticated() && !client->getNickname().empty() && !client->getUsername().empty())
    {
        server.sendMessage(client->getFd(), ":Server 462 :You may not reregister\r\n");
        return;
    }
    std::string params = msg.substr(5);
    std::vector<std::string> tokens = Utils::split(params, ' ');
    if (tokens.size() < 4)
    {
        server.sendMessage(client->getFd(), ":Server 461 :Need more parameters\r\n");
        return;
    }
    std::string username = tokens[0];
    std::string realname = tokens[3];
    if (username.empty() || realname.empty())
    {
        server.sendMessage(client->getFd(), ":Server 461 :Need more parameters\r\n");
        return;
    }
    client->setUsername(username);
    client->setRealname(realname);
    server.sendMessage(client->getFd(), ":Server 001 :User set\r\n");
}