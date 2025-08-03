#pragma once
#include "pch.h"
#include "Accounts/UserBusiness/Clients/ClientDetails.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"
class ClientMenu {
public:
	void displayClientMenu();

	ClientMenu(ClientManager& cliMgr, std::shared_ptr<ClientDetails> clientDet)
		: clientManager(cliMgr), clientDetails(clientDet) {
	}
private:
	bool deleteClient();
	bool updateClient();
	void clientStockManagement(); //menu// maybe time to use extension of the stock class
	void debtorMenu(); //needto make a money owing field, make stock first.
	void displayClientMap();
	static std::unordered_map<std::string, std::string> clientMap;
	ClientManager& clientManager;
	std::shared_ptr<ClientDetails> clientDetails;
};