#pragma once
#include "Accounts/UserBusiness/Clients/ClientManager.h"
#include "ClientMenu.h"
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/SetBusinessFromDB.h"
#include "Accounts/UserBusiness/BusinessDetails.h"
#include "Accounts/UserBusiness/Clients/ClientDetails.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include "StockMenu.h"

class BusinessMenu {
private:
	std::shared_ptr<User> currentUser;
	std::map<std::string, std::string> businessMap;
	MongoDBDataManager dbManager;
	BusinessDetails businessDetails;
	AccountManager& manager;
	BusinessManager& bizManager;
	ClientManager cliManager;
	std::shared_ptr<ClientDetails> clientDetails;
	ClientMenu clientMenu;
	StockMenu stockMenu;
	int maxBusinesses = 3;



	// Internal helpers
	void chooseBusiness();
	std::map<std::string, std::string> retrieveUsersBusinessIDs();
	bool validateUserBusiness(const std::string businessID);
	bool createClient();
	size_t userBusinessCount();
	void addSelfToBusinessID();
	bool validateMaxBusinesses();
	bool manageStock();

public:
	void setBusiness(const std::string businessID);
	void displayBusMenu();

	BusinessMenu(AccountManager& manager, BusinessManager& busManager)
		: currentUser(AccountManager::currentUser),
		businessMap(),
		dbManager(),
		businessDetails(manager),
		manager(manager),
		clientDetails(std::make_shared<ClientDetails>(manager, busManager)),
		bizManager(busManager), cliManager(dbManager), 
		clientMenu(cliManager, clientDetails), stockMenu(busManager, manager, dbManager){}
};
