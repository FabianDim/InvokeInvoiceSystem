#include "InvokeInvoiceSystem.h"

int main() {

	/*Main Executable*/
    //InvokeInvoiceSystem iis;
    //iis.run();

	/*Invoice Menu*/
    AccountManager accountManager;
    BusinessManager businessManager;
    MongoDBDataManager dbManager;
    ClientManager cliManager(dbManager);
    StockManager stkManager;

    auto user = std::make_shared<User>("test@email.com", "password123");
    user->setMongoUserID("USR00000001");
    accountManager.setTestUser(user);
    businessManager.setBusinessGlobally("BUS00000001");

    // Create context
    AppContext ctx{ accountManager, businessManager, dbManager, cliManager, stkManager };

    BusinessMenu businessMenu(ctx);
    InvoiceMenu invoiceMenu(ctx);
    invoiceMenu.setTestUser(user);
    invoiceMenu.displayMenu();

    

    //businessMenu.displayBusMenu();

    return 0;

}

//int main(int argc, char* argv[]) {
//	testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}


