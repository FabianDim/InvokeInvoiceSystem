#include "pch.h"  
#include <gtest/gtest.h>  
#include <gmock/gmock.h>  
#include "InvoiceMenu.h"  
#include "User.h"  

struct InvoiceMenuTestAccount : testing::Test {  
    User* user;  
    InvoiceMenuTestAccount() {  
        user = new User("email", "pass");  
    }  
    ~InvoiceMenuTestAccount() {  
        delete user;  
    }  
};  

//TEST_F(InvoiceMenuTestAccount, InvoiceMenuRuns) {  
//    InvoiceMenu invoiceMenu;  
//    invoiceMenu.setCurrentUser(std::make_shared<User>(user));  
//    invoiceMenu.displayMenu();
//
//}