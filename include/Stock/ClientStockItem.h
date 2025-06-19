#pragma once
#include "pch.h"  
//#include "Accounts/UserBusiness/ClientManager.h"
#include "Accounts/UserBusiness/BusinessManager.h"  
#include "System/Database/MongoDBDataManager.h"  
#include "StockItem.h"
#include "Stock/SetStockFromDB.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"  
#include "Accounts/UserBusiness/Clients/SetClientFromDB.h"  
class ClientStockItem : public StockManager {
private:
	float clientPrice;
	float profitMargin;
	bool updateClientDB(const float& price, const float& profMargin);
	std::shared_ptr<Client> curClient;
public: 
	float recalculateProfitMargin(float originalValue, float modifiedValue, float originalProfitMargin);
	void setPrice(float price);
	float getPrice();
	ClientStockItem(std::shared_ptr<Client> client, MongoDBDataManager& dbMgr)
		: StockManager(dbMgr), // Explicitly call the base class constructor  
		curClient(client){}
	bool updateAndAddClientStock();

};