#include "System/Menu/StockMenu.h"

void StockMenu::displayStockMenu() {
	//Search business stock
	//
	
	int choice = 0;
	std::cout << "\n========== Stock Management Menu ==========\n";
	std::cout << "1. Add new stock item\n";
	std::cout << "2. Search for a stock item\n";
	std::cout << "3. Return to main menu\n";
	std::cout << "Please enter your choice: ";
	std::cin >> choice;
	do {
		switch (choice) {
		case 1:
			stockDetails.collectStockInfo();
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			std::cout << "Returning to main menu...\n";
			break;
		default:
			std::cout << "Invalid choice. Please try again.\n";
			break;
		} 
	} while (choice != 6);
}