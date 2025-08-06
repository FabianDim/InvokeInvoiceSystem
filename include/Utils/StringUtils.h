#pragma once
#include <string>
#include <iostream>

inline std::string ansi256Colour(const std::string& text, int colourNum) {
    return "\033[38;5;" + std::to_string(colourNum) + "m" + text + "\033[0m";
}
inline std::string ansiRGBColour(const std::string& text, int r, int g, int b) {
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + text + "\033[0m";
}

inline std::string colourRed(const std::string& text) {
    return "\033[31m" + text + "\033[0m";
}
inline std::string colourBlue(const std::string& text) {
    return "\033[34m" + text + "\033[0m";
}
inline std::string colourYellow(const std::string& text) {
    return "\033[33m" + text + "\033[0m";
}
inline std::string colourLime(const std::string& text) {
    return "\033[92m" + text + "\033[0m";
}
inline std::string colourIndigo(const std::string& text) {
    // Try to use 256-colour or RGB for "indigo"; fallback to blue
    return ansi256Colour(text, 54); // 54 is indigo-ish
    // Or for RGB: return ansiRGBColour(text, 75, 0, 130);
}

inline std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}