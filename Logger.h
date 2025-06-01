#pragma once

#include <string>
#include <filesystem>

class Logger {
public:
    static void log(const std::string& message);
private:
    static void initializeLogFile();
    static std::string getLogFilePath();
};