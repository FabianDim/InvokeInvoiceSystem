#include "InvokeInvoiceSystem.h"

int main() {

	/*Main Executable*/
    //InvokeInvoiceSystem iis;
    //iis.run();

	/*Invoice Menu*/
    auto user = std::make_shared<User>("test@email.com", "password123");
    user->setMongoUserID("USR00000001");
    AccountManager accManager;
    accManager.setTestUser(user);         // Set this FIRST
    InvoiceMenu invoiceMenu;              // Now safe to construct
    invoiceMenu.chooseBusiness();

    return 0;

}

//int main(int argc, char* argv[]) {
//	testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}