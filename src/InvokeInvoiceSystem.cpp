#include "InvokeInvoiceSystem.h"

int main() {

	/*Main Executable*/
    //InvokeInvoiceSystem iis;
    //iis.run();

	/*Invoice Menu*/
    auto user = std::make_shared<User>("test@email.com", "password123");
    user->setMongoUserID("USR00000001");
    AccountManager accountManager;
    BusinessManager businessManager;
    accountManager.setTestUser(user);
    // Pass it to BusinessMenu
    BusinessMenu businessMenu(accountManager, businessManager);
    businessMenu.displayBusMenu();

    return 0;

}

//int main(int argc, char* argv[]) {
//	testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}


