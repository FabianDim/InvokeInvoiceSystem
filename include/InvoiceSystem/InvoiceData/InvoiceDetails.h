#pragma once

#include "pch.h"
#include "Accounts/AccountManager.h"
#include "System/Database/MongoDBDataManager.h"
#include "InvoiceSystem/InvoiceData/Invoice.h"
#include "Accounts/User.h"
#include "Stock/StockManager.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"
#include "Accounts/UserBusiness/BusinessManager.h"

enum class InvoiceStep {
    ENTER_INVOICE_ID,
    ENTER_DATE,
    ENTER_DUE,
    ENTER_CLIENT,
    ENTER_STOCK,
    ENTER_PAYMENT,
    CHOOSE_TEMPLATE,
    CONFIRM,
    DONE
};

const std::unordered_map<int, std::string> invoiceTemplate{
    {1, "Peece"}
};

class InvoiceDetails {
    friend class InvoiceMenu;

    struct InvoiceInput {
        std::string invoiceID;
        std::string clientInvoiceID;
        std::string invoiceDate;
        std::string dueDate;
        std::shared_ptr<Client> client;
        std::unordered_map<std::shared_ptr<StockItem>, int> stockQuantities;
        bool isPaid = false;
        bool gstIncluded = false;
        std::string invoiceTemplate;
        std::string notes;
    };

public:
    InvoiceDetails(
        AccountManager& accManager,
        MongoDBDataManager& dbMgr,
        BusinessManager& busManager,
        ClientManager& clientMgr,
        StockManager& stockManager)
        : accountManager(accManager),
        dbManager(dbMgr),
        bizManager(busManager),
        cliManager(clientMgr),
        stkMgr(stockManager) {
    }
    std::shared_ptr<Invoice> currentInvoice;
    // Input steps
    bool enterInvoiceID();
    bool enterDate();
    bool enterDueDate();
    bool selectClient();
    bool selectStock();
    bool chooseTemplate();
    bool enterPayment();
    bool confirmInfo();

    // Setup
    void collectInvoiceInfo();
    std::string makeInvoiceID();
    bsoncxx::document::value createInvoiceDoc();
    void insertInvoiceDoc(bsoncxx::document::value doc);

private:
    int thisInvoiceID = 0;
    InvoiceStep currentStep = InvoiceStep::ENTER_INVOICE_ID;
    InvoiceInput userInvoice;

    AccountManager& accountManager;
    MongoDBDataManager& dbManager;
    BusinessManager& bizManager;
    ClientManager& cliManager;
    StockManager& stkMgr;
    int stockQuantity();
    bool setCurrentInvoice();
};
