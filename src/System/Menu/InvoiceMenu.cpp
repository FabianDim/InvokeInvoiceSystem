#include "System/Menu/InvoiceMenu.h"
#include "Accounts/UserBusiness/SetBusinessFromDB.h"
void InvoiceMenu::displayMenu() {
	int choice;
	do {
		std::cout << colourBlue("Welcome to the invoice menu") << "\nYou are using " << colourLime(currentBusiness->getBizName()) << std::endl << std::endl <<
			"1. Create a new invoice\n2. View invoice history\n3. Edit or delete an invoice\n4. Change business \n5.Back to main menu " << std::endl;
		std::cout << "Please choose an option: ";
		std::cin >> choice;
		switch (choice) {
		case 1: {
			break;
		}
		case 2:
			// Do something
			break;
			// etc.
		case 3:
			break;
		case 4:
			break;
		default:
			std::cout << "Invalid choice. Please try again.\n";
			break;
		}

	} while (choice != 5);
	return;
}


