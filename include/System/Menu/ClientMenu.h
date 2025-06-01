#pragma once
#inclde "pch.h"
#include "Accounts/AccountManager.h"
class ClientMenu {
public:
	void displayMenu();
private:
	bool deleteClient();
	bool updateClient();
	std::unordered_map<std::string, std::string> getAllClientsNameAndID();  //ID , Name
	void clientStockManagement(); //menu// maybe time to use extension of the stock class
	void debtorMenu(); //needto make a money owing field, make stock first.

};