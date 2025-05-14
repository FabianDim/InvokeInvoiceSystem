#pragma once

#include "pch.h"
#include "User.h"
#include "MongoDBDataManager.h"
#include "MongoDBHandler.h"
#include "AccountManager.h"

class SetUser {

public:
	shared_ptr<User> setUserOnLogin(const string& email);
};