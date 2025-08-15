#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace Utils 
{
    std::string trim(const std::string& str);
    bool isValidNickname(const std::string& nick);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string intToString(int value);
}

#endif