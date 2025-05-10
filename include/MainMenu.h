#pragma once

#include <iostream>
#include "pch.h"
#include "InvoiceSystem.h"
#include "AccountManager.h"

class MainMenu {
	friend class AccountManager; // Allow AccountManager to access private members of MainMenu
public:
	void displayMenu(User* user);
	void handleUserInput(User* user);

	void exitProgram();
private:
	void createAccount();
	void login();


};