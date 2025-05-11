#include "InvoiceSystem.h"


void InvoiceSystem::printTitleBox() {
    //SetConsoleOutputCP(CP_UTF8); // For proper box character rendering

    // Use standard raw string (no u8 prefix)
    std::string line = R"(  
╔══════════════════════════════════════════════╗  
║                                              ║  
║  WELCOME TO THE INVOKE INVOICE SYSTEM (TM)   ║  
║                                              ║  
╚══════════════════════════════════════════════╝  
)";
    std::cout << line << std::endl;
}

void InvoiceSystem::startSystem() {
	MainMenu mainMenu;
    AccountManager accountManager;

    while (true) {
        mainMenu.displayMenu(accountManager.getAccount());
    }
}
