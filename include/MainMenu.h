#pragma once

#include <iostream>
#include "pch.h"
#include "InvoiceSystem.h"
#include "AccountManager.h"

class MainMenu {
	friend class AccountManager;
	friend class InvoiceSystem;// Allow AccountManager to access private members of MainMenu
public:
	void displayMenu(AccountManager& manager);
	void loggedInMenu(AccountManager& manager);
	static void printTitleBox();
private:
	void createAccount();
	void login();
	void isLoggedIn(AccountManager& accountManager);


};