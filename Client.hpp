#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int fd;
    bool isAuthenticated;
    std::string nickname;
    std::string username;
    std::string realname;

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
    ~Client()
    {

    }
};

#endif