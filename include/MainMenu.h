#pragma once

#include <iostream>
#include "pch.h"
#include "InvoiceSystem.h"
#include "AccountManager.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <BusinessDetails.h>

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