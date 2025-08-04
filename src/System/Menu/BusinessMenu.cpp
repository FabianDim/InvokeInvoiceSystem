#include "InvoiceSystem/System/Menu/BusinessMenu.h"

void BusinessMenu::displayBusMenu() {
	int choice;

	do {
		if (BusinessManager::getBusiness() == nullptr) { //hitting business is not set but it is. because get bizname works
			chooseBusiness();
		}
		std::cout << "\n========== '" << colourLime(BusinessManager::getBusiness()->getBizName())  << "' Business Menu ==========\n";
		std::cout << "1. Create a New Business (" << userBusinessCount() << " out of max " << maxBusinesses << ")" << std::endl; //one day change this to maximum owned because you might be able to add to more exisitng businesses
		std::cout << "2. Switch Active Business\n";
		std::cout << "3. Register Another Business\n";
		std::cout << "4. Manage Clients\n";
		std::cout << "5. Manage Business Stock\n";
		std::cout << "6. Manage businesses\n";
		std::cout << "7. Back to Main Menu\n";
		std::cout << "Please enter your choice: ";
		std::cin >> choice;
		std::cout << std::endl;

		switch (choice) {
		case 1:
			if(validateMaxBusinesses())businessDetails.collectBusinessInfo();
			break;
		case 2:
			chooseBusiness();
			break;
		case 3:
			addSelfToBusinessID();
			break;
		case 4:
			clientMenu.displayClientMenu();
			break;
		case 5:
			stockMenu.displayStockMenu();
			break;
		case 6:

			break;
		case 7:
			std::cout << "Returning to main menu...\n";
			break;
		default:
			std::cout << "Invalid choice. Please try again.\n";
			break;
		}
	} while (choice != 6);
}

	void BusinessMenu::chooseBusiness() {
		businessMap = retrieveUsersBusinessIDs();
		if (!businessMap.empty() && businessMap.size() == 1) {
			const auto& curBiz = *businessMap.begin();
			if (validateUserBusiness(curBiz.first)) {
				BusinessManager::setBusinessGlobally(curBiz.first);
			}
			return;
		}

		int choice;
		std::cout << "Please choose the business you'd like use and/or edit: \n";
		int i = 0;
		std::unordered_map<int, std::string> idMap;
		for (auto& curBiz : businessMap) {
			if (validateUserBusiness(curBiz.first)) {
				std::cout << i+1 << ": " << curBiz.second << std::endl;// make this find the businesses name not the business numbers
				idMap[i+1] = curBiz.first;
			}
			i++;
		}
		std::cout << i + 1 << ": " << "Create a new business" << std::endl;
		while (true) {
			std::cout << "Please choose the number of a business (* to go back): ";
			std::cin >> choice;

			if (std::to_string(choice) == "*") {
				return;
			}
			if (choice == i + 1) {
				if (validateMaxBusinesses())businessDetails.collectBusinessInfo();
				return;
			}
			if (idMap.contains(choice)) {
				
				BusinessManager::setBusinessGlobally(idMap[choice]);
			}
			else {
				std::cout << "Invalid choice, choose a valid number from 0 to " << i << ": " << std::endl;
				continue;
			}
			break;
		}
		return;
	}

	std::map<std::string, std::string> BusinessMenu::retrieveUsersBusinessIDs() {
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

	bool BusinessMenu::validateUserBusiness(const std::string& businessID) {
		auto result = dbManager.findOne("Users", make_document(kvp("UserID", AccountManager::currentUser->getMongoUserID()), kvp("BusinessIDs", businessID)));
		if (result) {
			return true;
		}
		return false;
	}

	bool BusinessMenu::createClient() {
		if (clientDetails) {
			clientDetails->collectClientInfo();
			return true;
		}
		else {
			std::cerr << "Error: clientDetails is not initialized.\n";
		}
		return false;
	}

	void BusinessMenu::setBusiness(const std::string businessID) {
		BusinessManager::setBusinessGlobally(businessID);
	}

	size_t BusinessMenu::userBusinessCount() {
		return retrieveUsersBusinessIDs().size();
	}

	void BusinessMenu::addSelfToBusinessID() {//adds a user to an existing business. Intergrate the bus number into more stats so the users actually know num
		std::cout << "Please specify the ID of the business for which you'd like to join (BUSXXXXXXXX): ";

		SetUser setUser;
		std::string biz;
		std::cin >> biz;

		bool success = false;

		do {
			success = businessDetails.addUserExistingBusiness(biz);
			if (!success && biz != "*") {
				std::cout << "Type in the ID of an existing business: ";
				std::cin >> biz;
			}
			else if (biz == "*") {
				return;
			}
		} while (!success);

		biz[0] = std::toupper(biz[0]);
		biz[1] = std::toupper(biz[1]);
		biz[2] = std::toupper(biz[2]);
		std::cout << "Added " << currentUser->getFirstName() << "to " << biz << std::endl;
		return;
	}

	bool BusinessMenu::validateMaxBusinesses() {
		if (userBusinessCount() >= maxBusinesses) {
			std::cout << colourRed("You have to many businesses, delete a business or upgrade your plan.\n") << std::endl;
			return false;
		}
		return true;
	}

	bool BusinessMenu::manageStock() {
		return false;
	}
