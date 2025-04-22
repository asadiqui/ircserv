#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <poll.h>

class Client; // Forward declaration

class ServerSocket 
{
private:
    int port;
    std::string password;
    int socketFd;
    struct sockaddr_in sockAddr;
    std::map<int, Client*> clients; // for client socket management

public:
    ServerSocket(int port, std::string password);
    ~ServerSocket();
    void newClient(struct pollfd* fds, int max_fds);
    void sender(int fd, const std::string& msg);
    int getSocketFd() const 
    { 
        return socketFd;
    }
};

#endif