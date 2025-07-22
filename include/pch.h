// pch.h
#pragma once  

// Include standard headers  
#include <iostream>  
#include <string>  
#include <memory>  
#include <unordered_map>  
#include <optional>  
#include <functional>
#include <string_view>

// Include MongoDB headers  
#include <mongocxx/client.hpp>  
#include <mongocxx/instance.hpp>  
#include <mongocxx/uri.hpp>  
#include <bsoncxx/builder/stream/document.hpp>  
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/value.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>  
#include "System/Database/MongoDBDataManager.h"
// Include bcrypt library  
#include "Accounts/PasswordHashing/bcrypt.h"  

//include pdf table
extern "C" {
#include <hpdf.h>    
}
#include <cstring>
#include <cstdlib>
// Add any other necessary includes here
#include <jansson.h>
#include <vector>

#include <map>

#include <algorithm>
#include <thread>
#include <fmt/core.h>
#include <cctype>
#include <set>
#include <unordered_set>

//custom utils
#include "Utils/StringUtils.h"
#include "Utils/TemplateUtil.h"

//manager classes
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"   // if you have a ClientManager in that folder
#include "Stock/StockManager.h"