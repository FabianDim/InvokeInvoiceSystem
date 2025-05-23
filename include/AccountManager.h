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
    std::string makeUserID();

    bool needsAccountSetup(const std::string& email);
    static std::shared_ptr<User> currentUser;
    bool hasUppers = false;
private:
    MongoDBDataManager dataManager;
    std::unordered_map<std::string, std::shared_ptr<User>> accounts;

};
