#include "System/Menu/StockMenu.h"

void StockMenu::displayStockMenu() {
	//Search business stock
	//
	
	int choice = 0;

	do {
		std::cout << "\n========== Stock Management Menu ==========\n";
		std::cout << "1. Add new stock item\n";
		std::cout << "2. Search for a stock item\n";
		std::cout << "5. Return to business menu\n";
		std::cout << "Please enter your choice: ";
		std::cin >> choice;
		switch (choice) {
		case 1:
			stockDetails.collectStockInfo();
			break;
		case 2:
			stockSearchFunction();
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			std::cout << "Returning to main menu...\n";
			return;
		default:
			std::cout << "Invalid choice. Please try again.\n";
			break;
		} 
	} while (choice != 6);
}

void StockMenu::stockSearchFunction() {
	std::cout << colourBlue("\nAll Business Stock:\n");
	std::unordered_map < int, std::string> resultMap;
	stkManager.displayAllStock();
	auto searchMapOpt = stkManager.createSearchMap(); 
	if (searchMapOpt.has_value()) {
		auto searchMap = searchMapOpt.value();
		std::string search;
		std::cout << "\nPlease search for the stock you'd like to manage \nEither use your pre-defined keywords or the items name or ID";
		do {
			std::cout << "\nSearch: ";
			std::cin >> search;
			if (search == "*") return;
			int count = 1;
			search = toLower(search);

			if (searchMap[search].empty()) {
				std::cout << "No results for that query" << std::endl;
				continue;
			}

			auto& stkMap = stkManager.stkMap;
			for (const auto& res : searchMap[search]) {
				resultMap[count] = res;
				std::cout << count << ": " << stkMap[res] << std::endl;
				count++;
			}

			std::cout << "\nType the number of an item to manage it \n('Search' to search again)" << std::endl;
			std::string choice = "";
			do {
				std::cin >> choice;
				if (toLower(choice) != "search" && resultMap.contains(stoi(choice))) {

					//std::cout << resultMap[stoi(choice)] << std::endl;
					StockManager::setStockItem(resultMap[stoi(choice)]);
					//std::cout << StockManager::getCurrentStockItem()->getName();
					displayItemMenu();
					return;
				}

			} while (toLower(choice) != "search");
		} while (search != "*");
	}

}

void StockMenu::displayItemMenu() {
	int choice = 0;
	
	do {
		std::cout << "\n\n========== " << colourLime(StockManager::getCurrentStockItem()->getName()) << " Item Menu ==========\n";
		std::cout << "1. Edit Item Details\n";
		std::cout << "2. Assign client pricing\n";
		std::cout << "3. Delete Item\n";
		std::cout << "5. Back to stock menu\n";
		std::cout << "Please enter your choice: ";
		std::cin >> choice;
		switch (choice) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			std::cout << "Returning to stock menu...\n";
			return;
		default:
			std::cout << "Invalid choice. Please try again.\n";
			break;
		}
	} while (choice != 6);
}
