#pragma once
#include "pch.h"
#include "User.h"
#include "SetUserFromDB.h"
#include "System/Menu/MainMenu.h"
#include "PasswordHashing/bcrypt.h"
#include "System/Database/MongoDBHandler.h"
#include "System/Database/MongoDBDataManager.h"
#include "Accounts/AccountServices/IAccountManager.h"



class AccountManager : public IAccountManager {
    friend class User;
    friend class MongoDBDataManager;
	friend class BusinessRepository;
public:
    AccountManager(MongoDBDataManager& dataManager) : dataManager(dataManager) {};
	~AccountManager() override = default;
    bool validEmail(const std::string& email) override;
    bool validName(const std::string& name) override;
    bool doesAccountExist(const std::string& username);
    bool doesPasswordMatch(const std::string& passweord);
    bool validatePassword(const std::string& password);
    void createAccount(std::string& userEmail, std::string& userPassword) override;
    void login() override;
    std::shared_ptr<User> getAccount();
    bool isLoggedIn() const override;
    void logOut() override;
    std::string makeUserID();
    
    void setTestUser(std::shared_ptr<User> user);

    bool needsAccountSetup(const std::string& email);
    static std::shared_ptr<User> currentUser;
    bool hasUppers = false;
private:
    MongoDBDataManager& dataManager;
    std::unordered_map<std::string, std::shared_ptr<User>> accounts;

};
