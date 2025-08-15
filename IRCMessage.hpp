#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <vector>

struct IRCMessage 
{
    std::string prefix;      // Optional: sender info
    std::string command;     // The IRC command (PRIVMSG, JOIN, etc.)
    std::vector<std::string> params;  // Command parameters
    std::string trailing;    // Optional: trailing parameter after ':'
    
    static IRCMessage parse(const std::string& rawMessage);
    std::string toString() const;
};

#endif
