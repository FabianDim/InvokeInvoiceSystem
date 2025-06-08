#pragma once
#include "../BusinessManager.h"
#include "pch.h"

#include "Client.h"

class ClientManager {
private:
	static std::shared_ptr<Client> currentClient;

public:
	static void setClient(std::shared_ptr<Client> client);
	static void clear();
	static std::unordered_map<std::string, std::string> fetchBizClients();
};