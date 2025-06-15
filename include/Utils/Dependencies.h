#pragma once

#pragma once

#include "pch.h"
#include "Accounts/UserBusiness/BusinessManager.h"

struct AppContext {
    AccountManager& accountMgr;
    BusinessManager& businessMgr; 
    MongoDBDataManager& dbMgr;
};