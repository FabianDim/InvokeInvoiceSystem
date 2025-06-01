#include "System/Menu/BusinessMenu.h"
void BusinessMenu::displayBusMenu() {
	int choice;
	do {
		std::cout << "\n========== '" << colourLime(currentBusiness->getBizName())  << "' Business Menu ==========\n";
		std::cout << "1. Create a New Business (" << ammountOfUserBusinesses() << " out of max 3)\n"; //one day change this to maximum owned because you might be able to add to more exisitng businesses
		std::cout << "2. Switch Active Business\n";
		std::cout << "3. Register Another Business\n";
		std::cout << "4. Manage Clients\n";
		std::cout << "5. Manage Business Stock\n";
		std::cout << "6. Back to Main Menu\n";
		std::cout << "Please enter your choice: ";
		std::cin >> choice;
		
		std::cout << std::endl;

		switch (choice) {
		case 1:
			businessDetails.collectBusinessInfo();
			break;
		case 2:
			chooseBusiness();
			break;
		case 3:
			addSelfToBusinessID();
			break;
		case 4:
			createClient();
			break;
		case 5:
			//manageStock();
			break;
		case 6:
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
				SetBusiness business;
				currentBusiness = business.setUpBusiness(curBiz.first);
			}
			return;
		}

		int choice;
		std::cout << "Please choose the business you'd like use and/or edit: \n";
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
				std::cout << "Invalid choice, choose a valid number from 0 to " << i - 1 << ": " << std::endl;
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

	bool BusinessMenu::validateUserBusiness(const std::string businessID) {
		auto result = dbManager.findOne("Users", make_document(kvp("UserID", AccountManager::currentUser->getMongoUserID()), kvp("BusinessIDs", businessID)));
		if (result) {
			return true;
		}
		return false;
	}

	bool BusinessMenu::createClient() {
		clientDetails.collectClientInfo();
		return false;
	}

	void BusinessMenu::setBusiness(const std::string businessID) {
		SetBusiness setBiz;
		currentBusiness = setBiz.setUpBusiness(businessID);
	}

	int BusinessMenu::ammountOfUserBusinesses() {
		return retrieveUsersBusinessIDs().size();
	}

	void BusinessMenu::addSelfToBusinessID() {
		std::cout << "Please specify the ID of the business for which you'd like to join (BUSXXXXXXXX): ";

		SetUser setUser;
		std::string biz;
		std::cin >> biz;

		while (!businessDetails.addUserExistingBusiness(biz)) {
			std::cout << colourRed("Business not found! Try again or type * to exit");
		}
		biz[0] = std::toupper(biz[0]);
		biz[1] = std::toupper(biz[1]);
		biz[2] = std::toupper(biz[2]);
		std::cout << "Added " << currentUser->getFirstName() << " to " << biz << std::endl;
		return;
	}
