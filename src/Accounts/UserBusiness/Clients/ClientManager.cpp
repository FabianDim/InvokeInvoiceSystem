#include "Accounts/UserBusiness/Clients/ClientManager.h"

std::shared_ptr<Client> ClientManager::currentClient = nullptr;
void ClientManager::setClient(std::shared_ptr<Client> client) {
	currentClient = client;
}
void ClientManager::clear() {
	currentClient = nullptr;
}

std::unordered_map<std::string, std::string> ClientManager::fetchBizClients() {
	try {
		MongoDBDataManager dbManager;
		if (BusinessManager::getBusiness() == nullptr) {
			std::cerr << "Business is not INITIALISED" << std::endl;
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
	return {};

}
