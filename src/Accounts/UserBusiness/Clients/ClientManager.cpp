#include "InvoiceSystem/Accounts/UserBusiness/Clients/ClientManager.h"
#include "InvoiceSystem/Accounts/UserBusiness/Clients/SetClientFromDB.h"
std::shared_ptr<Client> ClientManager::currentClient = nullptr;
void ClientManager::setClient(std::shared_ptr<Client> client) {
	currentClient = client;
}
std::shared_ptr<Client> ClientManager::getCurClient() {
	return currentClient;
}
void ClientManager::clear() {
	currentClient = nullptr;
}

std::unordered_map<std::string, std::string> ClientManager::fetchBizClients() {
	try {
		MongoDBDataManager dbManager;
		if (BusinessManager::getBusiness() == nullptr) {
			throw std::runtime_error("Business is not initialised");
			return {};
		}

		auto result = dbManager.findOne("Business", make_document(kvp("BusinessID", BusinessManager::getBusiness()->getBizID())));
		auto element = result.value()["ClientIDs"];
		if (!element || result.value()["ClientIDs"].get_array().value.empty()) {
			std::cout << "You have no clients. Make a client to get started." << std::endl;
			return {};
		}

		auto bsonArray = result.value()["ClientIDs"].get_array().value;
		size_t size = std::distance(bsonArray.begin(), bsonArray.end());

		std::unordered_map<std::string, std::string> clients;
		clients.reserve(size);

		for (const auto& elem : bsonArray) {
			//error with converting doc to char or something is often about using kvp
			auto filter = dbManager.findOne("Clients", make_document(kvp("ClientID", static_cast<std::string>(elem.get_utf8().value))));
			if (!filter) {
				std::cout << "Can't find clientID" << std::endl;
				return {};
			}
			auto cliName = filter->view()["ClientName"].get_utf8().value;
			clients[static_cast<std::string>(elem.get_utf8().value)] = cliName;
		}
		return clients;
	}
	catch (const mongocxx::exception& e) {
		std::cerr << e.what() << std::endl;
		return{};
	}
	catch (const std::runtime_error& e) {
		std::cerr << "Runtime error: " << e.what() << std::endl;
		
	}
	return {};

}

bool ClientManager::chooseAClient() {
	try {
		auto clients = fetchBizClients();
		if (clients.size() == 0) return false;
		int count = 1;
		std::unordered_map<int, std::string> choiceMap;
		for (auto& client : clients) {
			choiceMap[count] = client.first;
			std::cout << colourYellow(client.second) << " | " << client.first << std::endl;
			count++;
		}
		int choice = 0;
		do {
			std::cout << "Please choose a client you'd like to use: ";
			std::cin >> choice;
		} while (!choiceMap.contains(choice));
		if (SetClient::setClientFromDB(choiceMap[choice]) != nullptr) {
			currentClient = SetClient::setClientFromDB(choiceMap[choice]);
			return true;
		}
		return false;
	}
	catch (const std::out_of_range& e) {
		std::cerr << "Out of range error choosing client: " << e.what() << '\n';
	}
	catch (const std::exception& e) {
		std::cerr << "General std exception choosing client: " << e.what() << '\n';
	}
	catch (...) {
		std::cerr << "Unknown error occurred while choosing client.\n";
	}
}
