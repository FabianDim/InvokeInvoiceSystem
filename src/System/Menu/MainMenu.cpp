#include "MainMenu.h"
AccountManager accountManager;
void MainMenu::displayMenu(AccountManager& manager) {
    printTitleBox();
    int choice;
    do {
        std::cout << "\n1. Create Account\n2. Login\n3. Exit\nPlease select an option: ";
        std::cin >> choice;
        std::cin.ignore(); // Clear the newline character from the input buffer

        switch (choice) {
        case 1:
            manager.createAccount();
            break;
        case 2:
            manager.login();
            break;
        case 3:
            std::cout << "Goodbye!\n";
            break;
        default:
            std::cout << "Invalid option. Please try again.\n";
        }
    } while (choice != 3);
}
void MainMenu::loggedInMenu(AccountManager& manager) {
    printTitleBox();
    int choice;
    do {
        std::cout << "Welcome Back, " << manager.getAccount()->getFirstName() << endl;
        std::cout << "\n1. Create Account\n2. Login\n3. Exit\nPlease select an option: ";
        std::cin >> choice;
        std::cin.ignore(); // Clear the newline character from the input buffer

        switch (choice) {
        case 3:
            std::cout << "Goodbye!\n";
            break;
        default:
            std::cout << "Invalid option. Please try again.\n";
        }
    } while (choice != 3);
}
void MainMenu::printTitleBox() {
    //SetConsoleOutputCP(CP_UTF8); // For proper box character rendering

    // Use standard raw string (no u8 prefix)
    std::string line = R"(  
????????????????????????????????????????????????  
?                                              ?  
?  WELCOME TO THE INVOKE INVOICE SYSTEM (TM)   ?  
?                                              ?  
????????????????????????????????????????????????  
)";
    std::cout << line << std::endl;
}