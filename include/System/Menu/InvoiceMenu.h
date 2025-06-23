#pragma once  
#include "pch.h"  
#include "MainMenu.h"  
#include "Accounts/User.h"  
#include <memory> // Include this header for std::shared_ptr  
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/BusinessRepository.h"
#include "InvoiceSystem/PDF/InvoicePdfGenerator.h"
#include "InvoiceSystem/InvoiceData/InvoiceDetails.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"


class InvoiceMenu {  
public:
	InvoiceMenu(AccountManager& accManager) : currentUser(AccountManager::currentUser), 
		invoiceDetails(accManager), cliManager(dbManager) {}
	void setTestUser(std::shared_ptr<User> user) {
		testUser = user;
	}
	void displayMenu();
	
private:
	void createInvoice();
	std::shared_ptr<User> currentUser;
	std::map<std::string, std::string> businessMap;
	InvoicePdfGenerator generator;
	std::shared_ptr<User> testUser;
	MongoDBDataManager dbManager;
	InvoiceDetails invoiceDetails;
	ClientManager cliManager;
};