#include "IRCMessage.hpp"
#include "Utils.hpp"

IRCMessage IRCMessage::parse(const std::string& rawMessage) 
{
    IRCMessage msg;
    std::string message = Utils::trim(rawMessage);
    
    if (message.empty()) 
        return msg;
    
    size_t pos = 0;
    
    // Parse prefix (if exists, starts with ':')
    if (message[0] == ':') 
    {
        size_t spacePos = message.find(' ');
        if (spacePos != std::string::npos) 
        {
            msg.prefix = message.substr(1, spacePos - 1);
            pos = spacePos + 1;
        }
    }
    
    // Find trailing parameter (starts with ':')
    size_t trailingPos = message.find(" :", pos);
    std::string commandAndParams;
    
    if (trailingPos != std::string::npos) 
    {
        commandAndParams = message.substr(pos, trailingPos - pos);
        msg.trailing = message.substr(trailingPos + 2);
    } 
    else 
    {
        commandAndParams = message.substr(pos);
    }
    
    // Parse command and parameters
    std::vector<std::string> tokens = Utils::split(Utils::trim(commandAndParams), ' ');
    if (!tokens.empty()) 
    {
        msg.command = tokens[0];
        for (size_t i = 1; i < tokens.size(); ++i) 
        {
            if (!tokens[i].empty()) 
            {
                msg.params.push_back(tokens[i]);
            }
        }
    }
    
    return msg;
}

std::string IRCMessage::toString() const 
{
    std::string result;
    
    if (!prefix.empty()) 
    {
        result += ":" + prefix + " ";
    }
    
    result += command;
    
    for (size_t i = 0; i < params.size(); ++i) 
    {
        result += " " + params[i];
    }
    
    if (!trailing.empty()) 
    {
        result += " :" + trailing;
    }
    
    return result;
}
