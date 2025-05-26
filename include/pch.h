// pch.h
#pragma once  

// Include standard headers  
#include <iostream>  
#include <string>  
#include <memory>  
#include <unordered_map>  
#include <optional>  

// Include MongoDB headers  
#include <mongocxx/client.hpp>  
#include <mongocxx/instance.hpp>  
#include <mongocxx/uri.hpp>  
#include <bsoncxx/builder/stream/document.hpp>  
#include <bsoncxx/builder/basic/kvp.hpp>  

// Include bcrypt library  
#include "bcrypt.h"  

// Add any other necessary includes here
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <unordered_map>// Resolve ambiguity
#include <thread>
#include <fmt/core.h>
#include <cctype>
#include <set>
#include <unordered_set>

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}