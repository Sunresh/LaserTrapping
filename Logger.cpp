#include "Logger.h"
#include <iostream>
#include <fstream>
#include <ctime>

namespace {
    std::string logFilePath;
    bool isInitialized = false;
}

void Logger::initializeLogFile() {
    if (isInitialized) return;

    // Get the project root (assume current working directory)
    std::string projectRoot = std::filesystem::current_path().string();
    // Create a folder named "DesktopFolder" inside the project root
    std::string desktopFolder = projectRoot + "/DesktopFolder";
    // Create the folder if it doesn't exist
    std::filesystem::create_directories(desktopFolder);

    // Set the log file path
    logFilePath = desktopFolder + "/app.log";
    isInitialized = true;

    // Initialize the log file with a header
    std::ofstream logFile(logFilePath, std::ios::app);
    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr); // Store time in a variable
        logFile << "===== Log Started: " << std::ctime(&now) << "=====\n";
        logFile.close();
    }
}

std::string Logger::getLogFilePath() {
    if (!isInitialized) {
        initializeLogFile();
    }
    return logFilePath;
}

void Logger::log(const std::string& message) {
    // Ensure the log file is initialized
    if (!isInitialized) {
        initializeLogFile();
    }

    // Get current timestamp
    std::time_t now = std::time(nullptr); // Store time in a variable
    std::string timestamp = std::ctime(&now);
    timestamp.pop_back(); // Remove trailing newline from ctime

    // Write to file
    std::ofstream logFile(logFilePath, std::ios::app);
    if (logFile.is_open()) {
        logFile << "[" << timestamp << "] " << message << "\n";
        logFile.close();
    }
    else {
        std::cerr << "Error: Could not open log file: " << logFilePath << std::endl;
    }

    // Also output to console for debugging
    std::cout << "[LOG] [" << timestamp << "] " << message << std::endl;
}