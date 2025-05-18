#pragma once

#include "pch.h"
#include "User.h"
#include "MongoDBDataManager.h"
#include "MongoDBHandler.h"
#include "AccountManager.h"

class SetUser {

public:
	std::shared_ptr<User> setUserOnLogin(const std::string& email, const std::string& password);
};