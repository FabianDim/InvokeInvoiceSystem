#pragma once  
#include "pch.h"  
#include "MainMenu.h"  
#include "User.h"  
#include <memory> // Include this header for std::shared_ptr  
#include <AccountManager.h>

class InvoiceMenu {  
public:
	InvoiceMenu() : currentUser(AccountManager::currentUser) {}
	void setCurrentUser(std::shared_ptr<User> user) {
		currentUser = user;
	}
	void displayMenu();

private:
	std::shared_ptr<User> currentUser;
};