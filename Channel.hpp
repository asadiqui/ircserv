#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include "Client.hpp"

class Channel 
{
private:
    std::string name;
    std::string topic;
    std::set<int> members;        // Client file descriptors
    std::set<int> operators;      // Channel operators
    bool inviteOnly;
    bool topicRestricted;         // Only ops can change topic
    std::string key;              // Channel password
    int userLimit;                // 0 = no limit

public:
    Channel(const std::string& channelName);
    ~Channel();
    
    // Basic operations
    const std::string& getName() const { return name; }
    const std::string& getTopic() const { return topic; }
    void setTopic(const std::string& newTopic) { topic = newTopic; }
    
    // Member management
    bool addMember(int clientFd);
    bool removeMember(int clientFd);
    bool isMember(int clientFd) const;
    const std::set<int>& getMembers() const { return members; }
    size_t getMemberCount() const { return members.size(); }
    
    // Operator management
    bool addOperator(int clientFd);
    bool removeOperator(int clientFd);
    bool isOperator(int clientFd) const;
    
    // Channel modes
    bool isInviteOnly() const { return inviteOnly; }
    void setInviteOnly(bool value) { inviteOnly = value; }
    bool isTopicRestricted() const { return topicRestricted; }
    void setTopicRestricted(bool value) { topicRestricted = value; }
    const std::string& getKey() const { return key; }
    void setKey(const std::string& newKey) { key = newKey; }
    int getUserLimit() const { return userLimit; }
    void setUserLimit(int limit) { userLimit = limit; }
    
    // Utility
    bool isEmpty() const { return members.empty(); }
    std::string getMembersList(const std::map<int, Client*>& clients) const;
};

#endif
