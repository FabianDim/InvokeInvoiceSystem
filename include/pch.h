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
#include "Accounts/PasswordHashing/bcrypt.h"  

// Add any other necessary includes here

#include <vector>

#include <map>

#include <algorithm>
#include <thread>
#include <fmt/core.h>
#include <cctype>
#include <set>
#include <unordered_set>

#include "Utils/StringUtils.h"

