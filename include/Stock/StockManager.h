#pragma once  
#include "pch.h"  
#include "Accounts/UserBusiness/BusinessManager.h"  
#include "System/Database/MongoDBDataManager.h"  
#include "StockItem.h"
#include <unordered_map>  
#include <optional>  
#include <set> 
#include "Stock/SetStockFromDB.h"

class BusinessManager;

class StockManager {  
public:  
    StockManager(MongoDBDataManager& dbManager, BusinessManager& businessManager) :
        dbManager(dbManager), businessManager(businessManager) {}
    void displayAllStock();  
    std::optional<std::unordered_map<std::string, std::string>> stockMap();  
    std::optional<std::unordered_map<std::string, std::set<std::string>>> createSearchMap();
    std::unordered_map<std::string, std::string> stkMap;
    
    static void setStockItem(const std::string& stkID);
    static std::shared_ptr<StockItem> getCurrentStockItem();

private:  
    BusinessManager& businessManager;
    MongoDBDataManager& dbManager;
    static std::shared_ptr<StockItem> currentItem;
};