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
    bool validEmail(std::string& email);
    bool validName(std::string& name);
    bool doesAccountExist(const std::string& username);
    bool doesPasswordMatch(const std::string& password);
    bool validatePassword(const std::string& password);
    void createAccount();
    void login();
    std::shared_ptr<User> getAccount();
    bool isLoggedIn();
    void logOut();

    bool needsAccountSetup(const std::string& email);

    bool hasUppers = false;
private:
    std::unordered_map<std::string, std::shared_ptr<User>> accounts;
    std::shared_ptr<User> currentUser = nullptr;
};
