#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <map>
#include "Client.hpp"
#include "ClientManager.hpp"

class PollHandler;

class ServerSocket 
{
private:
    int port;
    std::string password;
    int socketFd;
    ClientManager clientManager;
public:
    ServerSocket(int port, std::string password);
    ~ServerSocket();
    int getSocketFd() 
    { 
        return socketFd; 
    }
    std::string getPassword() const
    { 
        return password; 
    }
    int getPort() const
    {
        return port;
    }
    void newClient(PollHandler& poller);
    bool sendMessage(int fd, const std::string& msg);
    std::map<int, Client*>& getClients()
    {
        return clientManager.getClients();
    }
};

#endif