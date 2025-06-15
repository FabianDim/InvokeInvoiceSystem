#include "Stock/StockManager.h"

std::shared_ptr<StockItem> StockManager::currentItem = nullptr;

void StockManager::displayAllStock() {
	auto curClients = stockMap();
	if (!curClients.has_value()) {
		std::cerr << colourRed("Failed to retrieve stock map.") << std::endl;
		return;
	}
	int count = 0;
	for (auto& [key, value] : curClients.value()) {
		std::cout<< value << " | " << key << std::endl;
	}
}

std::optional<std::unordered_map<std::string, std::string>> StockManager::stockMap() {
	auto business = BusinessManager::getBusiness();
	if (!business) {
		std::cerr << "No current business selected!\n";
		return std::nullopt;
	}
	std::unordered_map<std::string, std::string> stockMap;
	try {

		auto filter = dbManager.getCollection("Business")->find_one(make_document(kvp("BusinessID", business->getBizID())));
		auto array = filter->view()["StockIDs"].get_array().value;

		stockMap.reserve(std::distance(array.begin(), array.end()));

		for (auto& stk : array) {
			auto stkFilter = dbManager.findOne("Stock", make_document(kvp("StockID", stk.get_utf8().value)));
			std::string str{ stk.get_utf8().value };
			stockMap[str] = stkFilter->view()["Name"].get_utf8().value;
		}
		stkMap = stockMap;
		return stockMap;
	}
	catch (const mongocxx::exception& e) {
		std::cerr << colourRed(e.what()) << std::endl;
		return std::nullopt;
	}
	catch (const std::exception& e) {
		std::cerr << colourRed("Error: ") << colourRed(e.what()) << std::endl;
		return std::nullopt;
	}
	//catch(...){
	//	std::cerr << colourRed("Error creating stock map") << std::endl;
	//	return std::nullopt;
	//}

	return std::nullopt;
}

std::optional<std::unordered_map<std::string, std::set<std::string>>> StockManager::createSearchMap() {
	auto business = BusinessManager::getBusiness();
	if (!business) {
		std::cerr << "No current business selected!\n";
		return std::nullopt;
	}
	std::unordered_map<std::string, std::set<std::string>> searchMap;
	try {
		auto filter = dbManager.getCollection("Business")->find_one(make_document(kvp("BusinessID", business->getBizID())));
		auto array = filter->view()["StockIDs"].get_array().value;

		searchMap.reserve(std::distance(array.begin(), array.end()));
		std::cout << std::endl;
		for (auto& stkID : array) { //for each stock item in the businesses array.
			std::string str{ stkID.get_utf8().value };
			auto stkFilter = dbManager.findOne("Stock", make_document(kvp("StockID", str)));
			if (!stkFilter) {
				std::cerr << "StockID is invalid" << std::endl;
				return std::nullopt;
			}

			auto keywordArray = stkFilter->view()["ProductKeywords"].get_array().value;
			for (const auto& elem : keywordArray) {
				std::string keyword{ elem.get_utf8().value };
				searchMap[keyword].insert(str);
			}
		}
		return searchMap;
	}

	catch (const mongocxx::exception& e) {
		std::cerr << colourRed(e.what()) << std::endl;
		return std::nullopt;
	}
	catch (...) {
		std::cerr << colourRed("Error creating search map") << std::endl;
		return std::nullopt;
	}

	return std::nullopt;
}

std::shared_ptr<StockItem> StockManager::getCurrentStockItem() {
	if (currentItem != nullptr) {
		return currentItem;
	}
	std::cerr << colourRed("No Item Selected");
	return nullptr;
}

void StockManager::setStockItem(const std::string& stkID) {
	currentItem = SetStockItem::setStockItem(stkID);
}