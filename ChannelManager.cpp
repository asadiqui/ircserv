#include "ChannelManager.hpp"
#include "Utils.hpp"
#include <algorithm>

ChannelManager::ChannelManager() 
{
}

ChannelManager::~ChannelManager() 
{
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); 
         it != channels.end(); ++it) 
    {
        delete it->second;
    }
    channels.clear();
}

Channel* ChannelManager::createChannel(const std::string& name) 
{
    std::string normalizedName = Utils::normalizeChannelName(name);
    
    // Check if channel already exists
    std::map<std::string, Channel*>::iterator it = channels.find(normalizedName);
    if (it != channels.end()) 
    {
        return it->second;
    }
    
    // Create new channel with original name but store with normalized key
    Channel* newChannel = new Channel(name);
    channels[normalizedName] = newChannel;
    return newChannel;
}

Channel* ChannelManager::getChannel(const std::string& name) 
{
    std::string normalizedName = Utils::normalizeChannelName(name);
    std::map<std::string, Channel*>::iterator it = channels.find(normalizedName);
    if (it != channels.end()) 
    {
        return it->second;
    }
    return NULL;
}

bool ChannelManager::removeChannel(const std::string& name) 
{
    std::string normalizedName = Utils::normalizeChannelName(name);
    std::map<std::string, Channel*>::iterator it = channels.find(normalizedName);
    if (it != channels.end()) 
    {
        delete it->second;
        channels.erase(it);
        return true;
    }
    return false;
}

bool ChannelManager::channelExists(const std::string& name) const 
{
    std::string normalizedName = Utils::normalizeChannelName(name);
    return channels.find(normalizedName) != channels.end();
}

std::vector<std::string> ChannelManager::getChannelList() const 
{
    std::vector<std::string> channelNames;
    for (std::map<std::string, Channel*>::const_iterator it = channels.begin(); 
         it != channels.end(); ++it) 
    {
        channelNames.push_back(it->first);
    }
    return channelNames;
}

void ChannelManager::removeClientFromAllChannels(int clientFd) 
{
    std::vector<std::string> channelsToRemove;
    
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); 
         it != channels.end(); ++it) 
    {
        it->second->removeMember(clientFd);
        
        // If channel is empty, mark for removal
        if (it->second->isEmpty()) 
        {
            channelsToRemove.push_back(it->first);
        }
    }
    
    // Remove empty channels
    for (size_t i = 0; i < channelsToRemove.size(); ++i) 
    {
        removeChannel(channelsToRemove[i]);
    }
}

std::vector<std::string> ChannelManager::getClientChannels(int clientFd) const 
{
    std::vector<std::string> clientChannels;
    
    for (std::map<std::string, Channel*>::const_iterator it = channels.begin(); 
         it != channels.end(); ++it) 
    {
        if (it->second->isMember(clientFd)) 
        {
            clientChannels.push_back(it->first);
        }
    }
    
    return clientChannels;
}
