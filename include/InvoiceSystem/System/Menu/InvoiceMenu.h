#pragma once

#include "pch.h"
#include "MainMenu.h"
#include "InvoiceSystem/Accounts/User.h"
#include <memory>
#include "InvoiceSystem/Accounts/AccountManager.h"
#include "InvoiceSystem/Accounts/UserBusiness/BusinessManager.h"
#include "InvoiceSystem/Accounts/UserBusiness/BusinessRepository.h"
#include "InvoiceSystem/System/Database/MongoDBDataManager.h"
#include "InvoiceSystem/Accounts/UserBusiness/Clients/ClientManager.h"
#include "InvoiceSystem/InvoiceData/Stock/StockManager.h"
#include "InvoiceSystem/PDF/InvoicePdfGenerator.h"
#include "InvoiceSystem/InvoiceData/InvoiceDetails.h"
#include "InvoiceSystem/InvoiceManager.h"

class InvoiceMenu {
private:
    std::shared_ptr<User> currentUser;
    BusinessManager& bizManager;
    BusinessRepository& businessRepo;
    MongoDBDataManager& dbManager;
    ClientManager& cliManager;
    StockManager& stkMgr;
    InvoicePdfGenerator generator;
    InvoiceDetails invoiceDetails;
    std::shared_ptr<User> testUser;
    std::map<std::string, std::string> businessMap;

public:
    InvoiceMenu(
        AccountManager& accountMgr,
        BusinessManager& bizManager,
        BusinessRepository& businessRepo,
        MongoDBDataManager& dbMgr,
        ClientManager& cliManager,
        StockManager& stkMgr)
        : currentUser(accountMgr.getAccount()),
        bizManager(bizManager),
        businessRepo(businessRepo),
        dbManager(dbMgr),
        cliManager(cliManager),
        stkMgr(stkMgr),
        invoiceDetails(accountMgr, dbMgr, bizManager, cliManager, stkMgr), 
        generator(){
    }

    void setTestUser(std::shared_ptr<User> user) {
        testUser = user;
    }

    void displayMenu();

private:
    void createInvoice();
};
