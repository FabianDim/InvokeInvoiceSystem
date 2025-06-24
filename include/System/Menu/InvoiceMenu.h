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
	InvoiceMenu(AppContext& appCtx) : appCtx(appCtx), invoiceDetails(appCtx),
	cliManager(appCtx.dbMgr){}
	void setTestUser(std::shared_ptr<User> user) {
		testUser = user;
	}
	void displayMenu();
	
private:
	AppContext& appCtx;
	void createInvoice();
	std::shared_ptr<User> currentUser;
	std::map<std::string, std::string> businessMap;
	InvoicePdfGenerator generator;
	std::shared_ptr<User> testUser;
	InvoiceDetails invoiceDetails;
	ClientManager cliManager;
};