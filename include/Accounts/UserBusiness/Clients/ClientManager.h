#pragma once
#include "pch.h"

class Client;
class MongoDBDataManager;

class ClientManager {
private:
	static std::shared_ptr<Client> currentClient;
	MongoDBDataManager& dbManager;

public:
	ClientManager(MongoDBDataManager& db) : dbManager(db) {}

	static void setClient(std::shared_ptr<Client> client);
	static std::shared_ptr<Client> getCurClient();
	static void clear();

	static std::unordered_map<std::string, std::string> fetchBizClients();

	bool chooseAClient();
};
