#pragma once
#include "InvoiceSystem/Accounts/AccountManager.h" // Ensure AccountManager is included
#include "InvoiceSystem/Accounts/UserBusiness/BusinessDetails.h"
#include <iostream>
#include "pch.h"
#include <io.h>
#include <fcntl.h>

class MainMenu {
	friend class AccountManager;
	friend class System;// Allow AccountManager to access private members of MainMenu
public:

	void displayMenu(AccountManager& manager);
	void loggedInMenu(AccountManager& manager);
	static void printTitleBox();
	bool shouldExit = false;
private:
	MainMenu();
	void createAccount();
	void login();
	void isLoggedIn(AccountManager& manager);
	bool accountSetup(AccountManager& manager);


};