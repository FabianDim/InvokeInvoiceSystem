#include "InvoiceSystem/InvoiceData/Stock/SetStockFromDB.h"
std::shared_ptr<StockItem> SetStockItem::setStockItem(const std::string& stockID) {
	MongoDBDataManager dbManager;
	auto result = dbManager.findOne("Stock", make_document(kvp("StockID", stockID)));
	//auto stockDoc = dbManager.findOne("Business", make_document(kvp("BusinessID", BusinessManager::getBusiness()->getBizID())));
	try {
		if (result) {
			auto view = result->view();
			auto stockItem = std::make_shared<StockItem>(StockItem());

			//skip businessID and UserID array
			auto stockIDView = view["StockID"];
			auto businessIDView = view["BusinessID"];
			auto nameView = view["Name"];
			auto quantityView = view["Quantity"];
			auto stdPriceView = view["StdPrice"];
			auto profitMarginView = view["ProfitMargin"];

			std::string stockID{ stockIDView.get_utf8().value };
			std::string businessID{ businessIDView.get_utf8().value };
			std::string name{ nameView.get_utf8().value };
			int quantity{ quantityView.get_int32().value };
			double price{ stdPriceView.get_double().value };
			double profitMargin{ profitMarginView.get_double().value };


			stockItem->setStockID(stockID);
			stockItem->setBusinessID(businessID);
			stockItem->setName(name);
			stockItem->setStockOnHand(quantity);
			stockItem->setStdPrice(price);
			stockItem->setProfitMargin(profitMargin);

			return stockItem;
		}

		std::cerr << "No Stock found" << std::endl;
		return nullptr;
	}
	catch(mongocxx::exception& e ){
		std::cout << e.what();
		return nullptr;
	}
	catch (bsoncxx::exception& e) {
		// this exception is thrown, when a invalid json is parsed.
		std::cout << "Tried to pare a wrong json: " << std::endl;
		//e.waht() is returning a human readable message which says withe a drawed arrow, where the error occured.
		std::cout << e.what() << std::endl;
		std::cout << "------------------------------------- end error message" << std::endl;
	}
	return nullptr;
}

