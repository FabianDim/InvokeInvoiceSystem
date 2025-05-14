#pragma once
#include "pch.h"
#include "User.h"
#include "MainMenu.h"
#include "bcrypt.h"
#include "MongoDBHandler.h"
#include "MongoDBDataManager.h"
//using namespace std;
class AccountManager {
    friend class user; 
    friend class MongoDBDataManager;
public:
    AccountManager();
    bool validEmail(string& email);
    bool validName(string& name);
	bool doesAccountExist(const string& username); // we eventually want to hash the password.
    bool doesPasswordMatch(const string& password);
	bool validatePassword(const string& password);
    void createAccount();//create the account and store it.
    void login();//login
    shared_ptr<User>  getAccount();
    bool isLoggedIn();

private:
	unordered_map<string, shared_ptr<User>> accounts;
    shared_ptr<User> currentUser = nullptr;
};
