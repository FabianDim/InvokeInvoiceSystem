#pragma once
#include "Accounts/AccountManager.h" // Ensure AccountManager is included
#include "Accounts/userbusiness/BusinessDetails.h"
#include <iostream>
#include "pch.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
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