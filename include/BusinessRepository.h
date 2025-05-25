#pragma once
#include <pch.h>
#include <User.h>
#include <AccountManager.h>
class BusinessRepository {
public:
	std::vector<std::string> retrieveUsersBusinessIDs();
private:
	std::shared_ptr<User> currentUser;
};