#pragma once  
#include "pch.h"  
#include "MainMenu.h"  
#include "Accounts/User.h"  
#include <memory> // Include this header for std::shared_ptr  
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/BusinessRepository.h"
#include "InvoiceSystem/PDF/InvoicePdfGenerator.h"

class InvoiceMenu {  
public:
	InvoiceMenu() : currentUser(AccountManager::currentUser), currentBusiness(currentBusiness) {}
	void setTestUser(std::shared_ptr<User> user) {
		testUser = user;
	}
	void displayMenu();
	void chooseBusiness();
	std::map<std::string, std::string> retrieveUsersBusinessIDs();
	std::string getBusinessName(std::string businessID);
	bool validateUserBusiness(const std::string businessID);
	void setBusiness(const std::string businessID);
	
private:
	void createInvoice();
	std::shared_ptr<User> currentUser;
	InvoicePdfGenerator generator;
	std::shared_ptr<User> testUser;
	std::shared_ptr<BusinessRepository> currentBusiness;
	std::map<std::string, std::string> businessMap;
	MongoDBDataManager dbManager;
};