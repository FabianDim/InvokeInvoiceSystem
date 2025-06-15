#pragma once
#include "pch.h"  
#include "Accounts/UserBusiness/BusinessManager.h"  
#include "System/Database/MongoDBDataManager.h"  
#include "StockItem.h"
#include <unordered_map>  
#include <optional>  
#include <set> 
#include "Stock/SetStockFromDB.h"

class ClientStockItem : public StockItem {

};