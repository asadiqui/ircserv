#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client
{
private:
    int fd;
public:
    Client(int fd) : fd(fd) 
    {

    }
    int getFd() const
    { 
        return fd; 
    }
    ~Client() 
    {

    }
};

#endif