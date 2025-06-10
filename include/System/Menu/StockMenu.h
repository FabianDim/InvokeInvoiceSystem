#pragma once
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Stock/StockDetails.h"
#include "Accounts/AccountManager.h"
class StockMenu {
public:
	void displayStockMenu();
	StockMenu(BusinessManager& bizManager, AccountManager& accManager) : bizManager(bizManager), stockDetails(accManager) {}
private:
	BusinessManager& bizManager;
	StockDetails stockDetails;
	
};