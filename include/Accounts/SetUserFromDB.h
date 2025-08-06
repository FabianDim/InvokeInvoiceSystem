#pragma once

#include "../pch.h"
#include "System/Database/MongoDBDataManager.h"

class User;

class SetUser {
private:
	MongoDBDataManager mongoDataManager;
public:
	std::shared_ptr<User> setUserOnLogin(const std::string& email, const std::string& password);
	bool addBusinessToUser(const std::string& userID, const std::string& businessID);
};