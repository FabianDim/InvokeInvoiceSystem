#include "InvoiceSystem/Accounts/UserBusiness/SetBusinessFromDB.h"
std::shared_ptr<BusinessRepository> SetBusiness::setUpBusiness(const std::string businessID) {
	MongoDBDataManager dbManager;
	auto result = dbManager.findOne("Business", make_document(kvp("BusinessID", businessID)));
	if (result) {
		auto view = result->view();
		auto business = std::make_shared<BusinessRepository>(BusinessRepository());

		std::unordered_set<std::string> clients;
		std::vector<std::string> stock;

		//skip businessID and UserID array
		auto bizV = view["BusinessID"];
		auto abnV = view["ABN"];
		auto phoneV = view["Phone"];
		auto nameV = view["BusinessName"];
		auto addressV = view["BusinessAddress"];
		auto clientV = view["ClientIDs"];
		clients.reserve(std::distance(clientV.get_array().value.begin(), clientV.get_array().value.end()));
		auto stockV = view["StockIDs"];
		stock.reserve(std::distance(stockV.get_array().value.begin(), stockV.get_array().value.end()));
		auto acnV = view["ACN"];

		std::string bizID{ bizV.get_utf8().value };
		std::string abn{ abnV.get_utf8().value };
		std::string phone{ phoneV.get_utf8().value };
		std::string name{ nameV.get_utf8().value };
		std::string address{ addressV.get_utf8().value };
		std::string acn{ acnV.get_utf8().value };
		
		for (auto& val : stockV.get_array().value) {
			stock.push_back(static_cast<std::string>(val.get_utf8().value));
		}
		for (auto& val : clientV.get_array().value) {
			clients.insert(static_cast<std::string>(val.get_utf8().value));
		}
		business->setBizID(bizID);
		business->setAbn(abn);
		business->setPhone(phone);
		business->setBizName(name);
		business->setAcn(acn);
		business->setClients(clients);
		business->setStock(stock);

		return business;
	}
	std::cerr << "No business found" << std::endl;
	return nullptr;
}
