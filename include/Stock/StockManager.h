#pragma once
#include "pch.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include "System/Database/MongoDBDataManager.h"
class StockManager {
public:
	void displayAllStock();
	std::optional<std::unordered_map<std::string,std::string>> stockMap();
private:
	MongoDBDataManager& dbManager;
};