>>>>>>> Stashed changes
void MainMenu::displayMenu(AccountManager& manager) {
    printTitleBox();
    int choice;
    do {
        std::cout << "Hello, please choose an option below by typing the corresponding number and hitting enter.\n";
        std::cout << "\n1. Create Account\n2. Login\n3. Exit\nPlease select an option: ";
        std::cin >> choice;
        std::cout << std::endl;
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

        if (!accountSetup(manager)) break;

        std::cout << "Welcome back to the Invoke Invoice System, " << user->getFirstName() << std::endl;
        std::cout << "\n1. Invoice Menu\n2. Stock Management\
                       \n3. Account Management\n4. See invoices\n5. Logout\n6. Exit!\nPlease select an option: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        case 2:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        case 3:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        case 4:
            std::cout << "This feature is in development! Stay Tuned!\n\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        case 5:
            std::cout << "Logging out\n\n";
            manager.logOut();
            return;
        case 6:
            std::cout << "Goodbye!";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            shouldExit = true;
            break;
        default:
            std::cout << "Invalid option. Please try again.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    } while (choice != 7);
}
void MainMenu::printTitleBox() {
    SetConsoleOutputCP(CP_UTF8);
    std::string header =
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

bool MainMenu::accountSetup(AccountManager& manager) {
    auto user = manager.getAccount();
    if (manager.needsAccountSetup(user->getEmail())) {
        std::string input;
        std::cout << "\nIn order to start using the Invoke Invoice System, your account needs to be fully set up.\n";
        std::cout << "These details will help us autofill your invoices and make your workflow smoother.\n\n";

        std::cout << "First, we need to get to know your business a little better.\n";
        std::cout << "Let's go through a few quick questions about your ABN, business name, and contact info.\n";
        std::cout << "Don't worry — you'll only need to do this once.\n\n";

        std::cout << "Type 'Yes' to begin, 'No' to logout or the ID of an existing business to enter the business: \n**Businesses will be notified and permission needs to be granted** \n \nEnter your input:";

        std::cin >> input;

        std::transform(input.begin(), input.end(), input.begin(), ::tolower);

        BusinessDetails businessDetails(manager);
        if (input == "yes") {
            do {
                businessDetails.collectBusinessInfo();
            } while (manager.needsAccountSetup(user->getEmail()));
        }
        else if (input == "no") {
            manager.logOut();
            std::cout << std::endl;
            return false;
        }
        else if (input.size() > 3 && input.substr(0, 3) == "bus") {
            while (true) {
                if (businessDetails.validateBusiness(input)) {
                    businessDetails.addUserExistingBusiness(input);
                    break;
                }
                else {
                    std::cout << "Invalid business ID. Please enter a valid business (* to exit): ";
                    std::cin >> input;
                    if (input == "*") break;
                }
            }
        }
    }
    return true;
}
