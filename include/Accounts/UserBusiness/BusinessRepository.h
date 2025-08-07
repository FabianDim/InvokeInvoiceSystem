#pragma once
#include "pch.h"
#include "Accounts/User.h"
#include "Accounts/AccountManager.h"


class BusinessRepository {
    
public:
    explicit BusinessRepository(MongoDBDataManager& dbManager)
        : currentUser(AccountManager::currentUser)
        , dbManager(dbManager)
    {
        if (currentUser) {
            currentUserID = currentUser->getMongoUserID();
        }
        else {
            currentUserID.clear();
        }
    }
    const std::string& getBizID() const { return bizID; }
    const std::unordered_set<std::string>& getClients() const { return clients; }
    const std::vector<std::string>& getStock() const { return stock; }
    const std::string& getAbn() const { return abn; }
    const std::string& getPhone() const { return phone; }
    const std::string& getName() const { return name; }
    const std::string& getBizName() const { return bizName; }
    const std::string& getAddress() const { return address; }
    const std::string& getAcn() const { return acn; }

    // Setters
    void setBizID(const std::string& id) { bizID = id; }
    void setClients(const std::unordered_set<std::string>& values) { clients = values; } //maybe set size one day
    void setStock(const std::vector<std::string>& values) { stock = values; }
    void setAbn(const std::string& value) { abn = value; }
    void setPhone(const std::string& value) { phone = value; }
    void setName(const std::string& value) { name = value; }
    void setBizName(const std::string& value) { bizName = value; }
    void setAddress(const std::string& value) { address = value; }
    void setAcn(const std::string& value) { acn = value; }
private:
	std::string bizID;
	std::vector<std::string> stock;
    std::unordered_set<std::string> clients;
	std::string abn;
	std::string phone;
	std::string name;
	std::string bizName;
	std::string address;
	std::string acn;

    

    std::shared_ptr<User> currentUser;
    std::string currentUserID;
    MongoDBDataManager& dbManager;

};