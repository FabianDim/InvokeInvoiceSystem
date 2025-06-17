#pragma once
#include "../BusinessManager.h"
#include "pch.h"

#include "Client.h"

class ClientManager {
private:
	static std::shared_ptr<Client> currentClient;
	MongoDBDataManager& dbManager;
public:
	static void setClient(std::shared_ptr<Client> client);
	static std::shared_ptr<Client> getCurClient();
	static void clear();
	static std::unordered_map<std::string, std::string> fetchBizClients();
	ClientManager(MongoDBDataManager& dbMgr) : dbManager(dbMgr) {}
};