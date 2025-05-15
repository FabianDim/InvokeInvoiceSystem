#include "MainMenu.h"
void MainMenu::displayMenu(AccountManager& manager) {
    printTitleBox();
    int choice;
    do {
        cout << "Hello, please choose an option below by typing the corresponding number and hitting enter.\n";
        std::cout << "\n1. Create Account\n2. Login\n3. Exit\nPlease select an option: ";
        std::cin >> choice;
        std::cout << endl;
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
            shouldExit = true;
            break;
        default:
            std::cout << "Invalid option. Please try again.\n";
        }
    } while (choice != 3);
}
void MainMenu::loggedInMenu(AccountManager& manager) {
    printTitleBox();
    auto user = manager.getAccount();
    bool firstLogin;
    int choice;
    do {
        

        if (manager.needsAccountSetup(user->getEmail())) {
            std::cout << "Welcome to the Invoke Invoice System, " << user->getFirstName() << endl;
            string agree;
            std::cout << "Your account needs to be setup to utilise invoice functionality.\nPlease type 'yes' to begin or 'no' to logout : ";
            cin >> agree;
            if (agree == "no") {
                manager.logOut();
                return;
            }
        }
        std::cout << "Welcome back to the Invoke Invoice System, " << user->getFirstName() << endl;
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
            std::cout << "Goodbye!";
            this_thread::sleep_for(chrono::seconds(2));
            shouldExit = true;
            break;
        default:
            std::cout << "Invalid option. Please try again.\n";
            this_thread::sleep_for(chrono::seconds(2));
        }
    } while (choice != 7);
}
void MainMenu::printTitleBox() {
    static constexpr const char* header =
        "+-------------------------------------------------+\n"
        "|              INVOKE INVOICE SYSTEM              |\n"
        "+-------------------------------------------------+\n";
    std::cout << header;
}

void MainMenu::isLoggedIn(AccountManager& accountManager) {
    while (true && !shouldExit) {
        if (accountManager.getAccount() == nullptr) {
            displayMenu(accountManager);
        }
        else {
            loggedInMenu(accountManager);
        }
    }
}
