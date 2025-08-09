#pragma once

#include "../pch.h"
#include "Domain/Accounts/User.h"
#include "Application/Accounts/AccountManager.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"

class SetUser {
private:
	MongoDBDataManager& mongoDataManager;

public:

	SetUser(MongoDBDataManager& dataManager) : mongoDataManager(dataManager) {};

	std::shared_ptr<User> setUserOnLogin(const std::string& email, const std::string& password);
	bool addBusinessToUser(const std::string& userID, const std::string& businessID);
};