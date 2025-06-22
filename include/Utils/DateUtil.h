#pragma once
#include <ctime>
#include <iomanip>
#include <sstream>

std::string getCurrentDate() {
    std::time_t now = std::time(nullptr);               // get current time
    std::tm* localTime = std::localtime(&now);          // convert to local time

    std::ostringstream oss;
    oss << (1900 + localTime->tm_year) << "-"
        << std::setw(2) << std::setfill('0') << (1 + localTime->tm_mon) << "-"
        << std::setw(2) << std::setfill('0') << localTime->tm_mday;

    return oss.str(); // returns e.g., "2025-06-21"
}