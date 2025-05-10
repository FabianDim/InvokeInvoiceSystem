#pragma once
#include "pch.h"
#include "User.h"
#include "MainMenu.h"

//using namespace std;
class AccountManager {
    friend class user; 
public:
    AccountManager();
    bool validName(string& name);
	bool doesAccountExist(const string& username); // we eventually want to hash the password.
    bool doesPasswordMatch(const string& password);
	bool validatePassword(const string& password);
    void createAccount();//create the account and store it.
    void login();//login
    User* getAccount();
    bool isLoggedIn();

private:
	unordered_map<string, string> accounts;
    User* currentUser = nullptr;
};
