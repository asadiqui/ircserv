#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

class Client; // Forward declaration

class ServerSocket 
{
private:
    int port;
    std::string password;
    int socketFd;
    struct sockaddr_in sockAddr;
    Client* currentClient;
public:
    ServerSocket(int port, std::string password);
    ~ServerSocket();
    void run();
    void sender(int fd, const std::string& msg);
    int getSocketFd() const
    {
        return socketFd;
    }
};

#endif