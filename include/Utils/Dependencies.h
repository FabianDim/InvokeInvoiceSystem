#pragma once

#pragma once

#include "pch.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"
#include "Stock/StockManager.h"
//#include "InvoiceSystem/InvoiceData/InvoiceManager.h"
struct AppContext {
    AccountManager& accountMgr;
    BusinessManager& businessMgr; 
    MongoDBDataManager& dbMgr;
    ClientManager& cliManager;
    StockManager& stkMgr;
    //InvoiceManager& invoiceMgr;
};