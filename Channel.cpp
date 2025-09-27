#include "Channel.hpp"
#include <algorithm>

Channel::Channel(const std::string& channelName) 
    : name(channelName), inviteOnly(false), topicRestricted(true), userLimit(0)
{
}

Channel::~Channel() 
{
}

bool Channel::addMember(int clientFd) 
{

    if (userLimit > 0 && members.size() >= static_cast<size_t>(userLimit)) 
    {

        return false; // Channel is full
    }

    // key
    std::pair<std::set<int>::iterator, bool> result = members.insert(clientFd);
    
    // If this is the first member, make them an operator
    if (members.size() == 1) 
    {
        operators.insert(clientFd);
    }
    
    return result.second;
}

bool Channel::removeMember(int clientFd) 
{
    operators.erase(clientFd);
    return members.erase(clientFd) > 0;
}

bool Channel::isMember(int clientFd) const 
{
    return members.find(clientFd) != members.end();
}

bool Channel::addOperator(int clientFd) 
{
    if (!isMember(clientFd)) 
    {
        return false;
    }
    return operators.insert(clientFd).second;
}

bool Channel::removeOperator(int clientFd) 
{
    return operators.erase(clientFd) > 0;
}

bool Channel::isOperator(int clientFd) const 
{
    return operators.find(clientFd) != operators.end();
}

std::string Channel::getMembersList(const std::map<int, Client*>& clients) const 
{
    std::string membersList;
    
    for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) 
    {
        std::map<int, Client*>::const_iterator clientIt = clients.find(*it);
        if (clientIt != clients.end() && clientIt->second) 
        {
            if (!membersList.empty()) 
            {
                membersList += " ";
            }
            
            // Add operator prefix if applicable
            if (isOperator(*it)) 
            {
                membersList += "@";
            }
            
            membersList += clientIt->second->getNickname();
        }
    }
    
    return membersList;
}
