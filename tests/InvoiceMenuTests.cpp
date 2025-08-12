#include "pch.h"  
#include <gtest/gtest.h>  
#include <gmock/gmock.h>  

#include "View/LegacyMenus/InvoiceMenu.h"
#include "Application/Accounts/AccountManager.h"
#include "Domain/Accounts/User.h"  
#include "Application/Business/BusinessRepository.h"


struct InvoiceMenuTestAccount : testing::Test {  
    std::shared_ptr<User> user;  
    InvoiceMenuTestAccount() {  
        user = std::make_shared<User>("email", "pass");
        user->setMongoUserID("USR00000001");
    }  
    ~InvoiceMenuTestAccount() {  
    }  
};  

//TEST_F(InvoiceMenuTestAccount, InvoiceMenuRuns) {
//    AccountManager accmgr;  
//    accmgr.setTestUser(user);
//    BusinessRepository repo;
//    std::string bus = "BUS00000001";
//    bool associated = repo.isUserAssociatedWithBusiness(bus);
//
//    ASSERT_TRUE(associated);
//}