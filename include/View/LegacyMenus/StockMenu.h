#pragma once
#include "Application/Business/BusinessManager.h"
#include "Domain/Stock/StockDetails.h"
#include "Application/Accounts/AccountManager.h"
#include "Application/Stock/StockManager.h"
#include "Domain/Stock/ClientStockItem.h"
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