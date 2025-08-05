#pragma once
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Stock/StockDetails.h"
#include "Accounts/AccountManager.h"
#include "Stock/StockManager.h"
#include "Stock/ClientStockItem.h"
class StockMenu {
	friend class StockManager;
public:
	void displayStockMenu();
	StockMenu(BusinessManager& bizManager, AccountManager& accManager,MongoDBDataManager& dbManager) : 
		bizManager(bizManager), stockDetails(accManager), stkManager(stkManager), cliStockItem(client, dbManager, bizManager) {}
private:
	std::shared_ptr<Client> client = ClientManager::getCurClient();
	BusinessManager& bizManager;
	StockDetails stockDetails;
	StockManager stkManager;
	void stockSearchFunction();
	void displayItemMenu();
	ClientStockItem cliStockItem;
};