#include "MainMenu.h"
AccountManager accountManager;
void MainMenu::displayMenu(User* user) {
	InvoiceSystem::printTitleBox();
	if (user == nullptr) {
		std::cout << "1. Create Account" << std::endl;
		cout << "2. Login" << std::endl;
		std::cout << "3. Exit" << std::endl;
		handleUserInput(user);
	}
	else {
		cout << "Welcome Back " << user->getUsername() << endl;
		std::cout << "1. Create Account" << std::endl;
		std::cout << "3. Exit" << std::endl;
		handleUserInput(user);//update profiles
	}

}

void MainMenu::handleUserInput(User* user) {
	int choice = 0;

	std::cout << "Please select an option: ";
	std::cin >> choice;
	if (user == nullptr) {
		switch (choice) {//case of invalid input
		case 1:
			createAccount();
			break;
		case 2:
			login();
			break;
		}
	}
}

void MainMenu::createAccount() {

	accountManager.createAccount();
}

void MainMenu::login() {
	accountManager.login();
}
