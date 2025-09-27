#include "Logger.hpp"
#include "CommandParser.hpp"
#include <iostream>

void Logger::error(const std::string& msg) 
{
    std::cerr << "[ERROR] " << msg << std::endl;
}

void Logger::info(const std::string& msg)
{
    std::cout << "[INFO] " << msg << std::endl;
}