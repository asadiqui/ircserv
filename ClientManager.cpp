#include "ClientManager.hpp"

void ClientManager::addClient(int fd, Client* client)
{
    clients[fd] = client;
}

void ClientManager::removeClient(int fd)
{
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it != clients.end())
    {
        delete it->second;
        clients.erase(it);
    }
}

Client* ClientManager::getClient(int fd)
{
    std::map<int, Client*>::iterator it = clients.find(fd);
    if (it != clients.end())
    {
        return it->second;
    }
    return NULL;
}

std::map<int, Client*>& ClientManager::getClients()
{
    return clients;
}
Client* ClientManager::getClientByNickname(const std::string& nickname) {
    std::map<int, Client*>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        Client* client = it->second;
        if (client && client->getNickname() == nickname) {
            return client; // found
        }
    }
    return NULL; // not found
}