#pragma once

#include "../pch.h"
#include "Accounts/User.h"
#include "Accounts/AccountManager.h"

class SetUser {

public:
	std::shared_ptr<User> setUserOnLogin(const std::string& email, const std::string& password);
};