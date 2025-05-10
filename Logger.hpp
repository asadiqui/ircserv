#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

class Logger 
{
public:
    static void error(const std::string& msg);
    static void info(const std::string& msg);
};

#endif