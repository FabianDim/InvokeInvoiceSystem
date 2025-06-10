#include "Stock/StockManager.h"

void StockManager::displayAllStock() {

}

std::optional<std::unordered_map<std::string, std::string>> StockManager::stockMap() {
	auto business = BusinessManager::getBusiness();
	std::unordered_map<std::string, std::string> stockMap;
	try {
		auto filter = dbManager.findOne("Business", make_document(kvp("BusinessID", business->getBizID())));
		auto array = filter->view()["StockIDs"].get_array().value;

		stockMap.reserve(std::distance(array.begin(), array.end()));

		for (auto& stk : array) {
			auto stkFilter = dbManager.findOne("Stock", make_document(kvp("StockID", stk.get_utf8().value)));
			std::string str{ stk.get_utf8().value };
			stockMap[str] = stkFilter->view()["Name"].get_utf8().value;
		}
		return stockMap;
	}
	catch (const mongocxx::exception& e) {
		std::cerr << colourRed(e.what()) << std::endl;
		return std::nullopt;
	}
	catch(...){
		std::cerr << colourRed("Unidentified Error") << std::endl;
		return std::nullopt;
	}

	return std::unordered_map<std::string, std::string>();
}
