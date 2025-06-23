#pragma once
#include "pch.h"
#include "Accounts/AccountManager.h"
#include "System/Database/MongoDBDataManager.h"
#include "InvoiceSystem/InvoiceData/Invoice.h"
#include "Accounts/User.h"


enum class InvoiceStep {
    ENTER_INVOICE_ID,
    ENTER_DATE,
    ENTER_DUE,
    ENTER_CLIENT,
    ENTER_STOCK,
    ENTER_PAYMENT,
    CONFIRM,
    DONE
};

class InvoiceDetails {
    friend class InvoiceMenu;

    struct InvoiceInput {
        std::string invoiceID; // Optional; user can override
        std::string invoiceDate;
        std::string dueDate;
        std::shared_ptr<Client> client;
        std::unordered_map<std::string, int> stockQuantities;
        bool isPaid = false;
        bool gstIncluded = false;
        std::string notes;
    };

public:
    InvoiceDetails(AccountManager& accountManager) :
        cliManager(dbManager), accountManager(accountManager){}
    std::string toLower(std::string text);

    // Input steps
    bool enterInvoiceID();
    bool enterDate();
    bool enterDueDate();
    bool selectClient();
    bool selectStock();
    bool enterPayment();
    bool confirmInfo();

    // Setup
    void collectInvoiceInfo();
    std::string makeInvoiceID();
    bsoncxx::document::value createInvoiceDoc();
    void insertInvoiceDoc(bsoncxx::document::value doc);
private:
    int thisInvoiceID;
    InvoiceStep currentStep = InvoiceStep::ENTER_INVOICE_ID;
    InvoiceInput userInvoice;
    MongoDBDataManager dbManager;
    ClientManager cliManager;
    AccountManager& accountManager;
    bool setCurrentInvoice();
    
};
