#pragma once  
#include "pch.h"  
#include "Accounts/UserBusiness/BusinessManager.h"  
#include "System/Database/MongoDBDataManager.h"  
#include <unordered_map>  
#include <optional>  
#include <set>  

class StockManager {  
public:  
    StockManager(MongoDBDataManager& dbMgr) : dbManager(dbMgr) {}  
    void displayAllStock();  
    std::optional<std::unordered_map<std::string, std::string>> stockMap();  
    std::optional<std::unordered_map<std::string, std::set<std::string>>> createSearchMap();  

private:  
    MongoDBDataManager& dbManager;  
};