#pragma once
#include "pch.h"  
//#include "Application/Business/Clients/ClientManager.h"
#include "Application/Business/BusinessManager.h"  
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"  
#include "Domain/Stock/StockItem.h"
#include "Application/Stock/SetStockFromDB.h"
#include "Application/Business/Clients/ClientManager.h"  
#include "Application/Business/Clients/SetClientFromDB.h"  
class ClientStockItem : public StockManager {
private:
	float clientPrice;
	float profitMargin;
	bool updateClientDB(const float& price, const float& profMargin);
	std::shared_ptr<Client> curClient;
	MongoDBDataManager& dbMgr;
	BusinessManager& bizManager;
public: 
	float recalculateProfitMargin(float originalValue, float modifiedValue, float originalProfitMargin);
	void setPrice(float price);
	float getPrice();

	bool updateAndAddClientStock();
	ClientStockItem(std::shared_ptr<Client> client, MongoDBDataManager& dbMgr, BusinessManager& bizManager)
		: dbMgr(dbMgr), bizManager(bizManager), StockManager(dbMgr, bizManager), // Explicitly call the base class constructor  
		curClient(client) {
	}
};