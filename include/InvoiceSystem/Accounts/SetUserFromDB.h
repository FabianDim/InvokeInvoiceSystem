#pragma once

#include "../pch.h"
#include "InvoiceSystem/Accounts/User.h"
#include "InvoiceSystem/Accounts/AccountManager.h"
#include "InvoiceSystem/System/Database/MongoDBDataManager.h"

class SetUser {
private:
	MongoDBDataManager mongoDataManager;
public:
	std::shared_ptr<User> setUserOnLogin(const std::string& email, const std::string& password);
	bool addBusinessToUser(const std::string& userID, const std::string& businessID);
};