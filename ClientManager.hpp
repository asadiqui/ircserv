#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include <map>
#include "Client.hpp"

class ClientManager
{
private:
    std::map<int, Client*> clients;
public:
    void addClient(int fd, Client* client);
    void removeClient(int fd);
    Client* getClient(int fd);
    std::map<int, Client*>& getClients();
};

#endif