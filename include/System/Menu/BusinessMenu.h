#pragma once
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/SetBusinessFromDB.h"
#include "Accounts/UserBusiness/BusinessDetails.h"
#include "Accounts/UserBusiness/Clients/ClientDetails.h"

class BusinessMenu {
private:

	std::shared_ptr<User> currentUser;
	std::shared_ptr<BusinessRepository> currentBusiness;
	std::map<std::string, std::string> businessMap;
	MongoDBDataManager dbManager;
	BusinessDetails businessDetails;
	ClientDetails clientDetails;
	void chooseBusiness();
	std::map<std::string, std::string> retrieveUsersBusinessIDs();
	std::string getBusinessName(std::string businessID);
	bool validateUserBusiness(const std::string businessID);
	bool createClient();
	int ammountOfUserBusinesses();
	void addSelfToBusinessID();
public:
	void setBusiness(const std::string businessID);
	BusinessMenu(AccountManager& manager) : currentUser(AccountManager::currentUser), currentBusiness(currentBusiness), businessDetails(manager), clientDetails(manager) {};
	void displayBusMenu();
};