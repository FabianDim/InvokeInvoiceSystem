#pragma once
#include "Accounts/UserBusiness/Clients/ClientManager.h"
#include "ClientMenu.h"
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/SetBusinessFromDB.h"
#include "Accounts/UserBusiness/BusinessDetails.h"
#include "Accounts/UserBusiness/Clients/ClientDetails.h"
#include "Accounts/UserBusiness/BusinessManager.h"

class BusinessMenu {
private:
	std::shared_ptr<User> currentUser;
	std::map<std::string, std::string> businessMap;
	MongoDBDataManager dbManager;
	BusinessDetails businessDetails;
	AccountManager& manager;
	std::shared_ptr<ClientDetails> clientDetails;
	BusinessManager& bizManager;
	ClientManager cliManager;
	ClientMenu clientMenu;

	int maxBusinesses = 3;



	// Internal helpers
	void chooseBusiness();
	std::map<std::string, std::string> retrieveUsersBusinessIDs();
	bool validateUserBusiness(const std::string businessID);
	bool createClient();
	size_t userBusinessCount();
	void addSelfToBusinessID();
	bool validateMaxBusinesses();

public:
	void setBusiness(const std::string businessID);
	void displayBusMenu();

	BusinessMenu(AccountManager& manager, BusinessManager& busManager)
		: currentUser(AccountManager::currentUser),
		businessMap(),
		dbManager(),
		businessDetails(manager),
		manager(manager),
		clientDetails(std::make_shared<ClientDetails>(manager, busManager)), bizManager(busManager) {}
};
