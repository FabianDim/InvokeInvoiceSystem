#pragma once

#include "pch.h"
#include "ClientMenu.h"
#include "Accounts/UserBusiness/BusinessDetails.h"
#include "Accounts/UserBusiness/Clients/ClientDetails.h"
#include "StockMenu.h"

class BusinessMenu {
    friend class BusinessManager;

private:
    std::shared_ptr<User> currentUser;
    std::map<std::string, std::string> businessMap;
    BusinessDetails businessDetails;
    std::shared_ptr<ClientDetails> clientDetails;
    ClientMenu clientMenu;
    StockMenu stockMenu;
    MongoDBDataManager& dbManager;
    int maxBusinesses = 3;

    // Internal helpers
    void chooseBusiness();
    std::map<std::string, std::string> retrieveUsersBusinessIDs();
    bool validateUserBusiness(const std::string& businessID);
    bool createClient();
    size_t userBusinessCount();
    void addSelfToBusinessID();
    bool validateMaxBusinesses();
    bool manageStock();

public:
    BusinessMenu(
        AccountManager& accountMgr,
        BusinessManager& businessMgr,
        ClientManager& cliManager,
        MongoDBDataManager& dbMgr)
        : currentUser(accountMgr.getAccount()),
        businessDetails(accountMgr),
        clientDetails(std::make_shared<ClientDetails>(accountMgr, businessMgr)),
        clientMenu(cliManager, clientDetails),
        stockMenu(businessMgr, accountMgr, dbMgr),
        dbManager(dbMgr) {
    }

    void setBusiness(const std::string businessID);
    void displayBusMenu();
};
