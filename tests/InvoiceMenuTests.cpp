#include "pch.h"  
#include <gtest/gtest.h>  
#include <gmock/gmock.h>  
<<<<<<< Updated upstream
#include "InvoiceMenu.h"  
#include <AccountManager.h>
#include "User.h"  
#include <BusinessRepository.h>
=======
#include "System/Menu/InvoiceMenu.h"
#include "Accounts/AccountManager.h"
#include "Accounts/User.h"  
#include "Accounts/UserBusiness/BusinessRepository.h"
>>>>>>> Stashed changes

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