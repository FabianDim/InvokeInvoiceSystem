#pragma once

#include "../pch.h"
#include "Accounts/User.h"
#include "Accounts/AccountManager.h"
#include "System/Database/MongoDBDataManager.h"

class SetUser {
private:
	MongoDBDataManager& mongoDataManager;

public:

	SetUser(MongoDBDataManager& dataManager) : mongoDataManager(dataManager) {};

	std::shared_ptr<User> setUserOnLogin(const std::string& email, const std::string& password);
	bool addBusinessToUser(const std::string& userID, const std::string& businessID);
};