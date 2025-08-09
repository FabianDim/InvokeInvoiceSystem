#include "InvokeInvoiceSystem.h"
//int main() {
//
//	/*Main Executable*/
//    //InvokeInvoiceSystem iis;
//    //iis.run();
//
//	/*Invoice Menu*/
//    AccountManager accountManager;
//    BusinessManager businessManager;
//    MongoDBDataManager dbManager;
//    ClientManager cliManager(dbManager);
//    StockManager stkManager(dbManager,businessManager);
//
//
//    auto user = std::make_shared<User>("test@email.com", "password123");
//    user->setMongoUserID("USR00000001");
//
//    accountManager.setTestUser(user);
//    businessManager.setBusinessGlobally("BUS00000001");
//    BusinessRepository bizRepo;
//    // Create context
//
//    BusinessMenu businessMenu(accountManager, businessManager, cliManager, dbManager);
//    InvoiceMenu invoiceMenu(accountManager, businessManager, 
//        bizRepo, dbManager, cliManager, stkManager);
//    invoiceMenu.setTestUser(user);
//    invoiceMenu.displayMenu();
//
//    
//
//    //businessMenu.displayBusMenu();
//
//    return 0;
//
//}

//int main(int argc, char* argv[]) {
//	testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}

int main(int argc, char* argv[]) {
    MongoDBDataManager dataManager;
    AccountManager accountManager(dataManager);
    QApplication app(argc, argv);
    App::Views::MainWindow window(accountManager);
    window.resize(window.frameSize());
    window.show();
    return app.exec();
}
