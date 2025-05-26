#pragma once  
#include "pch.h"  
#include "MainMenu.h"  
#include "User.h"  
#include <memory> // Include this header for std::shared_ptr  
#include <AccountManager.h>
#include <BusinessRepository.h>

class InvoiceMenu {  
public:
	InvoiceMenu() : currentUser(AccountManager::currentUser), currentBusiness(currentBusiness) {}
	void setTestUser(std::shared_ptr<User> user) {
		testUser = user;
	}
	void displayMenu();
	void chooseBusiness();
	std::set<std::string> retrieveUsersBusinessIDs();
	std::string getBusinessName(std::string businessID);
	bool isUserAssociatedWithBusiness(const std::string businessID);
private:
	std::shared_ptr<User> currentUser;
	std::shared_ptr<User> testUser;
	BusinessRepository& currentBusiness;

	MongoDBDataManager dbManager;
};