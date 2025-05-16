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
    int choice;
    do {
        if (manager.needsAccountSetup(user->getEmail())) {
            string input;
            std::cout << "\n🔔 In order to start using the Invoke Invoice System, your account needs to be fully set up.\n";
            std::cout << "These details will help us autofill your invoices and make your workflow smoother.\n\n";

            std::cout << "👋 First, we need to get to know your business a little better.\n";
            std::cout << "Let's go through a few quick questions about your ABN, business name, and contact info.\n";
            std::cout << "Don't worry — you'll only need to do this once.\n\n";

            cout << "Type 'Yes' to begin";

            cin >> input;
            for (auto c : input) {
                tolower(c);
            }
            BusinessDetails businessDetails;
            if (input == "yes") {
                while (manager.needsAccountSetup(user->getEmail())) {
                    businessDetails.collectBusinessInfo();
                }
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
    SetConsoleOutputCP(CP_UTF8);
    string header =
        "╒═════════════════════════════════════════════════╕\n"
        "|              INVOKE INVOICE SYSTEM              |\n"
        "╘═════════════════════════════════════════════════╛\n";
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

void MainMenu::accountSetup(AccountManager& manager) {
    auto user = manager.getAccount();


}

