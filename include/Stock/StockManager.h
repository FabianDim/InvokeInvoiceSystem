#pragma once  
#include "pch.h"  
#include <unordered_map>
#include <optional>
#include <set>

class BusinessManager;
class MongoDBDataManager;
class StockItem;

class StockManager {  
public:  
    StockManager(MongoDBDataManager& dbManager, BusinessManager& businessManager) :
        businessManager(businessManager), dbManager(dbManager) {}
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