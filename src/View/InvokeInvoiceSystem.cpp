#include "View/InvokeInvoiceSystem.h"

// int main() {
//
//	/*Main Executable*/
//     //InvokeInvoiceSystem iis;
//     //iis.run();
//
//	/*Invoice Menu*/
//     AccountManager accountManager;
//     BusinessManager businessManager;
//     MongoDBDataManager dbManager;
//     ClientManager cliManager(dbManager);
//     StockManager stkManager(dbManager,businessManager);
//
//
//     auto user = std::make_shared<User>("test@email.com", "password123");
//     user->setMongoUserID("USR00000001");
//
//     accountManager.setTestUser(user);
//     businessManager.setBusinessGlobally("BUS00000001");
//     BusinessRepository bizRepo;
//     // Create context
//
//     BusinessMenu businessMenu(accountManager, businessManager, cliManager, dbManager);
//     InvoiceMenu invoiceMenu(accountManager, businessManager,
//         bizRepo, dbManager, cliManager, stkManager);
//     invoiceMenu.setTestUser(user);
//     invoiceMenu.displayMenu();
//
//
//
//     //businessMenu.displayBusMenu();
//
//     return 0;
//
// }

// int main(int argc, char* argv[]) {
//	testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
// }

int main(int argc, char* argv[]) {
    MongoDBDataManager data_manager;
    AccountManager account_manager(data_manager);
    QApplication app(argc, argv);
    Invoke::Application::Auth::QSettingsSessionManager session_manager;
    QCoreApplication::setOrganizationName("Invoke");
    QCoreApplication::setApplicationName("InvokeInvoiceSystem");
    App::Views::MainWindow window(account_manager);
    window.resize(800, 800);
    window.show();
    return app.exec();
}
