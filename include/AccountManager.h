#pragma once
#include "pch.h"
#include "User.h"
#include "MainMenu.h"
#include "bcrypt.h"
#include "MongoDBHandler.h"
#include "MongoDBDataManager.h"
#include "SetUserFromDB.h"
//using namespace std;
class AccountManager {
    friend class user; 
    friend class MongoDBDataManager;
public:
    AccountManager();
    bool validEmail(string& email);
    bool validName(string& name);
	bool doesAccountExist(const string& username);
    bool doesPasswordMatch(const string& password);
	bool validatePassword(const string& password);
    void createAccount();//create the account and store it.
    void login();//login
    shared_ptr<User>  getAccount();
    bool isLoggedIn();
    void logOut();

    bool needsAccountSetup(const string& email);

    bool hasUppers = false;
private:
	unordered_map<string, shared_ptr<User>> accounts;
    shared_ptr<User> currentUser = nullptr;
};
