#include "MainMenu.h"
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
            return;
        case 2:
            manager.login();
            return;
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
    auto user = manager.getAccount();
    int choice;
    do {
        std::cout << "Welcome Back, " << user->getFirstName() << endl;

        std::cout << "\n1. Create a new Invoice\n2. Invoice Management\n3. Stock Management\
                       \n4. Account Management\n5. See invoices\n6. Logout\n7. Exit!\nPlease select an option: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        case 2:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        case 3:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        case 4:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        case 5:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        case 6:
            std::cout << "Logging out\n\n";
            manager.logOut();
            return;
        case 7:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        default:
            std::cout << "Invalid option. Please try again.\n";
            this_thread::sleep_for(chrono::seconds(2));
        }
    } while (choice != 3);
}
void MainMenu::printTitleBox() {
    static constexpr const char* header =
        "+-------------------------------------------------+\n"
        "|     WELCOME TO THE INVOKE INVOICE SYSTEM        |\n"
        "+-------------------------------------------------+\n";
    std::cout << header;
}

void MainMenu::isLoggedIn(AccountManager& accountManager) {
    while (true) {
        if (accountManager.getAccount() == nullptr) {
            displayMenu(accountManager);
        }
        else {
            loggedInMenu(accountManager);
        }
    }
}
