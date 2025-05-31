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
			if (businessMap.size() == 1) {
				std::cout << "You don't have more businesses. Go to the business menu to add more.\n" << std::endl;
			}
			else {
				chooseBusiness();
			}
			break;
		}

	} while (choice != 5);
	return;
}

void InvoiceMenu::chooseBusiness() {
	 businessMap = retrieveUsersBusinessIDs();
	if (!businessMap.empty() && businessMap.size() == 1) {
		const auto& curBiz = *businessMap.begin();
		if (validateUserBusiness(curBiz.first)) {
			SetBusiness business;
			currentBusiness = business.setUpBusiness(curBiz.first);
		}
		displayMenu();
		return;
	}

	int choice;
	std::cout << "Please choose the business you'd like to create or manage invoices for: ";
	int i = 0;
	std::unordered_map<int, std::string> idMap;
	for (auto& curBiz : businessMap) {
		if (validateUserBusiness(curBiz.first)) {
			std::cout << i << ": " << curBiz.second << std::endl;// make this find the businesses name not the business numbers
			idMap[i] = curBiz.first;
		}
		i++;
	}
	while (true) {
		std::cout << "Please choose the number of a business (* to go back): ";
		std::cin >> choice;

		if (std::to_string(choice) == "*") {
			return;
		}
		if (idMap.contains(choice)) {
			SetBusiness business;
			currentBusiness = business.setUpBusiness(idMap[choice]);
		}
		else {
			std::cout << "Invalid choice, choose a valid number from 0 to " << i-1 << ": " << std::endl;
			continue;
		}
		break;
	}
	displayMenu();
	return;
}

std::map<std::string, std::string> InvoiceMenu::retrieveUsersBusinessIDs() {
	try {
		if (currentUser == nullptr) {
			std::cerr << "CurrentUser is not INITIALISED" << std::endl;
			return {};
		}
		auto result = dbManager.findOne("Users", make_document(kvp("UserID", currentUser->getMongoUserID())));
		if (!result) {
			std::cerr << "Array not found!" << std::endl;
			return {};
		}
		auto doc = result->view();
		auto arr = doc["BusinessIDs"].get_array().value;
		for (auto& elem : arr) {
            std::string business{ elem.get_utf8().value };
			auto res = dbManager.findOne("Business", make_document(kvp("BusinessID", business)));
			if (res) {
				auto doc = res->view();
				std::string businessName{ std::string(doc["BusinessName"].get_utf8().value) };
				businessMap[business] = businessName;
			}
		}
		return businessMap;
	}
	catch (const mongocxx::exception& e) {
		std::cerr << e.what() << std::endl;
		return{};
	}
	return {};

}

bool InvoiceMenu::validateUserBusiness(const std::string businessID) {
	auto result = dbManager.findOne("Users", make_document(kvp("UserID", AccountManager::currentUser->getMongoUserID()), kvp("BusinessIDs", businessID)));
	if (result) {
		return true;
	}
	return false;
}
