#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <map>
#include "Client.hpp"

class ServerSocket
{
private:
    int port;
    std::string password;
    int socketFd;
    std::map<int, Client*> clients;
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
    void newClient(int epoll_fd);
    void sender(int fd, const std::string& msg);
    std::map<int, Client*>& getClients()
    {
        return clients;
    }
};

#endif