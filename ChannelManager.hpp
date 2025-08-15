#ifndef CHANNELMANAGER_HPP
#define CHANNELMANAGER_HPP

#include <map>
#include <string>
#include <vector>
#include "Channel.hpp"

class ChannelManager 
{
private:
    std::map<std::string, Channel*> channels;

public:
    ChannelManager();
    ~ChannelManager();
    
    // Channel management
    Channel* createChannel(const std::string& name);
    Channel* getChannel(const std::string& name);
    bool removeChannel(const std::string& name);
    bool channelExists(const std::string& name) const;
    
    // Utility
    std::vector<std::string> getChannelList() const;
    void removeClientFromAllChannels(int clientFd);
    std::vector<std::string> getClientChannels(int clientFd) const;
    
    // Broadcasting
    void broadcastToChannel(const std::string& channelName, const std::string& message, 
                           int excludeFd = -1) const;
};

#endif
