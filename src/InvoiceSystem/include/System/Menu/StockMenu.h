#pragma once
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Stock/StockDetails.h"
#include "Accounts/AccountManager.h"
#include "System/Database/MongoDBDataManager.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"
#include "Stock/StockManager.h"
#include "Stock/ClientStockItem.h"
class StockMenu {
	friend class StockManager;
public:
	void displayStockMenu();
        StockMenu(BusinessManager& bizManager, AccountManager& accManager, MongoDBDataManager& dbManager) :
                bizManager(bizManager),
                stockDetails(accManager),
                stkManager(dbManager, bizManager),
                cliStockItem(client, dbManager, bizManager) {}
private:
	std::shared_ptr<Client> client = ClientManager::getCurClient();
	BusinessManager& bizManager;
	StockDetails stockDetails;
	StockManager stkManager;
	void stockSearchFunction();
	void displayItemMenu();
	ClientStockItem cliStockItem;
};