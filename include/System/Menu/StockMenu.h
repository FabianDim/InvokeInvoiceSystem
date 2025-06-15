#pragma once
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Stock/StockDetails.h"
#include "Accounts/AccountManager.h"
#include "Stock/StockManager.h"
class StockMenu {
	friend class StockManager;
public:
	void displayStockMenu();
	StockMenu(BusinessManager& bizManager, AccountManager& accManager,MongoDBDataManager& dbManager) : 
		bizManager(bizManager), stockDetails(accManager), stkManager(dbManager) {}
private:
	BusinessManager& bizManager;
	StockDetails stockDetails;
	StockManager stkManager;
	void stockSearchFunction();
	void displayItemMenu();
};