#pragma once

#include "pch.h"
#include "View/LegacyMenus/MainMenu.h"
#include "Domain/Accounts/User.h"
#include <memory>
#include "Application/Accounts/AccountManager.h"
#include "Application/Business/BusinessManager.h"
#include "Application/Business/BusinessRepository.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include "Application/Business/Clients/ClientManager.h"
#include "Application/Stock/StockManager.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include "Domain/Invoices/InvoiceDetails.h"
#include "Application/Invoices/InvoiceManager.h"

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
