#include "Application/Business/BusinessManager.h"

std::shared_ptr<BusinessRepository> BusinessManager::currentBusiness = nullptr;

void BusinessManager::setBusinessGlobally(const std::string bizID) {
	SetBusiness setBusiness;
	currentBusiness = setBusiness.setUpBusiness(bizID);
}

std::shared_ptr<BusinessRepository> BusinessManager::getBusiness() {
	if (currentBusiness == nullptr) {
		//std::cerr << "Business is not set." << std::endl;
		return nullptr;
	}
	return currentBusiness;
}

bool BusinessManager::chooseBusiness() {
	auto businessMap = retrieveUsersBusinessIDs();
	if (!businessMap.empty() && businessMap.size() == 1) {
		const auto& curBiz = *businessMap.begin();
		if (validateUserBusiness(curBiz.first)) {
			BusinessManager::setBusinessGlobally(curBiz.first);
		}
		return true;
	}

	int choice;
	std::cout << "Please choose the business you'd like use and/or edit: \n";
	int i = 0;
	std::unordered_map<int, std::string> idMap;
	for (auto& curBiz : businessMap) {
		if (validateUserBusiness(curBiz.first)) {
			std::cout << i + 1 << ": " << curBiz.second << std::endl;// make this find the businesses name not the business numbers
			idMap[i + 1] = curBiz.first;
		}
		i++;
	}
	while (true) {
		std::cout << "Please choose the number of a business (* to go back): ";
		std::cin >> choice;

		if (std::to_string(choice) == "*") {
			return false;
		}
		if (idMap.contains(choice)) {

			BusinessManager::setBusinessGlobally(idMap[choice]);
			return true;
		}
		else {
			std::cout << "Invalid choice, choose a valid number from 0 to " << i << ": " << std::endl;
			continue;
		}
		break;
	}
	return false;
}

std::map<std::string, std::string> BusinessManager::retrieveUsersBusinessIDs() {
	std::map<std::string, std::string> businessMap;
	try {
		if (AccountManager::currentUser == nullptr) {
			std::cerr << "CurrentUser is not INITIALISED" << std::endl;
			return {};
		}
		auto result = dbManager.findOne("Users", make_document(kvp("UserID", AccountManager::currentUser->getMongoUserID())));
		if (!result) {
			std::cerr << "Array not found!" << std::endl;
			return {};
		}
		auto doc = result->view();
		auto arr = doc["BusinessIDs"].get_array().value;
		for (auto& elem : arr) {
			std::string business{ elem.get_string().value };
			auto res = dbManager.findOne("Business", make_document(kvp("BusinessID", business)));
			if (res) {
				auto doc = res->view();
				std::string businessName{ std::string(doc["BusinessName"].get_string().value) };
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

bool BusinessManager::validateUserBusiness(const std::string businessID) {
	auto result = dbManager.findOne("Users", make_document(kvp("UserID", AccountManager::currentUser->getMongoUserID()), kvp("BusinessIDs", businessID)));
	if (result) {
		return true;
	}
	return false;
}