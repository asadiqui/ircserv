#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include "Utils.hpp"

class Client
{
private:
    int fd;
    bool isAuthenticated;
    std::string nickname;
    std::string username;
    std::string realname;
    std::set<std::string> channels;  // Channels this client is in

public:
    Client(int fd) : fd(fd), isAuthenticated(false)
    {

    }
    int getFd() const
    {
        return fd;
    }
    bool getIsAuthenticated() const
    { 
        return isAuthenticated;
    }
    void setAuthenticated(bool auth)
    { 
        isAuthenticated = auth;
    }
    std::string getNickname() const
    { 
        return nickname; 
    }
    void setNickname(const std::string& nick)
    { 
        nickname = nick;
    }
    std::string getUsername() const
    {
        return username;
    }
    void setUsername(const std::string& user)
    { 
        username = user;
    }
    std::string getRealname() const
    {
        return realname; 
    }
    void setRealname(const std::string& real)
    { 
        realname = real;
    }
    
    // Channel management
    void addChannel(const std::string& channelName)
    {
        channels.insert(Utils::normalizeChannelName(channelName));
    }
    void removeChannel(const std::string& channelName)
    {
        channels.erase(Utils::normalizeChannelName(channelName));
    }
    bool isInChannel(const std::string& channelName) const
    {
        return channels.find(Utils::normalizeChannelName(channelName)) != channels.end();
    }
    const std::set<std::string>& getChannels() const
    {
        return channels;
    }
    
    ~Client()
    {

    }
};

#endif