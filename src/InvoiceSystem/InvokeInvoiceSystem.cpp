#include "InvokeInvoiceSystem.h"
#include <QApplication>  
#include <QMainWindow>
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

#include <QApplication>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QLabel label("Qt6 Widgets Loaded!");
    label.setAlignment(Qt::AlignCenter);
    label.resize(300, 100);
    label.show();

    return app.exec();
}
