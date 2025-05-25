#include "InvokeInvoiceSystem.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>    
#include <InvoiceMenu.h>

int main() {

	/*Main Executable*/
    InvokeInvoiceSystem iis;
    iis.run();

	/*Invoice Menu*/
    //auto user = std::make_shared<User>("test@email.com", "password123");

    //InvoiceMenu invoiceMenu;
    //invoiceMenu.setCurrentUser(user);

    //invoiceMenu.displayMenu();

    return 0;

}

//int main(int argc, char* argv[]) {
//	testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}