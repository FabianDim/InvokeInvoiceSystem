#pragma once  
#include "pch.h"  
#include "MainMenu.h"  
#include "User.h"  
#include <memory> // Include this header for std::shared_ptr  

class InvoiceMenu {  
public:
	InvoiceMenu() : currentUser(AccountManager::currentUser) {}

	void displayMenu();

private:
	std::shared_ptr<User> currentUser;
};