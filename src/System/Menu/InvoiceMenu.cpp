#include "InvoiceMenu.h"
void InvoiceMenu::displayMenu() {
	int choice;
	do {
		std::cout << "Welcome to the Invoice Menu";
		std::cout << "1. Create a new invoice" << std::endl;
		std::cin >> choice;
		switch (choice) {
		case 1:
			// Do something
			break;
		case 2:
			// Do something
			break;
			// etc.
		}
	} while (choice != 5);
}

void InvoiceMenu::chooseBusiness() {
	int choice;
	std::cout << "Please choose the business you'd like to create or manage invoices for: ";
	int i = 0;
	std::unordered_map<int, std::string> idMap;
	for (auto& curBiz : retrieveUsersBusinessIDs()) {
		if (isUserAssociatedWithBusiness(curBiz)) {
			std::cout << i+1 << ": " << getBusinessName(curBiz) << std::endl;// make this find the businesses name not the business numbers
			idMap[i+1] = curBiz;
		}
		i++;
	}
	while (true) {
		std::cout << "Please choose the number of a business (* to go back): ";
		std::cin >> choice;

		if (std::to_string(choice) == "*") {
			return;
		}
		BusinessRepository* newBus = new BusinessRepository(idMap[choice]);
		currentBusiness = newBus;//problem
	}

}

std::set<std::string> InvoiceMenu::retrieveUsersBusinessIDs() {
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
		std::set<std::string> businesses;
		for (auto& elem : arr) {
			std::string business{ std::string(elem.get_utf8().value) };
			businesses.insert(business);
		}
		return businesses;
	}
	catch (const mongocxx::exception& e) {
		std::cerr << e.what() << std::endl;
		return{};
	}
	return {};

}

std::string InvoiceMenu::getBusinessName(std::string businessID) {
	auto result = dbManager.findOne("Business", make_document(kvp("BusinessID", businessID)));

	auto doc = result->view();
	std::string businessName{ std::string(doc["BusinessName"].get_utf8().value) };
	return businessName;
}

bool InvoiceMenu::isUserAssociatedWithBusiness(const std::string businessID) {
	auto result = dbManager.findOne("Users", make_document(kvp("UserID", AccountManager::currentUser->getMongoUserID()), kvp("BusinessIDs", businessID)));
	if (result) {
		return true;
	}
	return false;
}
