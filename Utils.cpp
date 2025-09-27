#include "Utils.hpp"
#include <sstream>

std::string Utils::trim(const std::string& str)
{
    std::string::size_type start = 0, end = str.length();

    while (start < end && (str[start] == ' ' || str[start] == '\t')) 
        start++;
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t'))
        end--;
    return str.substr(start, end - start);
}

bool Utils::isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false; 
    char first = nick[0];
    if (!std::isalpha(first) && 
        first != '[' && first != ']' && first != '{' && first != '}' &&
        first != '\\' && first != '|' && first != '^' && first != '`' && first != '_')
        return false;
    
    for (size_t i = 1; i < nick.length(); ++i) 
    {
        char c = nick[i];
        if (!std::isalnum(c) && c != '-' &&
            c != '[' && c != ']' && c != '{' && c != '}' &&
            c != '\\' && c != '|' && c != '^' && c != '`' && c != '_')
            return false;
    }
    return true;
}


bool Utils::isValidUsername(const std::string& username)
{
    for (size_t i = 0; i < username.length(); ++i) 
    {
        char c = username[i];
        if (c == ' ' || c == '!' || c == ':' || c == '#' || c == '@') 
            return false;
    }
    return true;
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string::size_type start = 0, end;

    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

std::string Utils::intToString(int value) 
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string Utils::normalizeChannelName(const std::string& channelName)
{
    std::string normalized = channelName;
    for (size_t i = 0; i < normalized.length(); ++i)
    {
        if (normalized[i] >= 'A' && normalized[i] <= 'Z')
        {
            normalized[i] = normalized[i] + ('a' - 'A');
        }
    }
    return normalized;
}