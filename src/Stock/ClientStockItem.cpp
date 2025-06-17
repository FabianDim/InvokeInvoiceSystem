#include "Stock/ClientStockItem.h"

bool ClientStockItem::updateClientDB(const float& price, const float& profMargin) {
    try {
        MongoDBDataManager dbManager;
        if (BusinessManager::getBusiness() == nullptr) {
            std::cerr << "Business is not INITIALISED in updateClientDB" << std::endl;
            return false;
        }
        auto result = dbManager.findOne("Clients", make_document(kvp("ClientID", ClientManager::getCurClient()->getClientID())));
        auto element = result.value()["ClientStockIDs"];
        auto doc_to_push = bsoncxx::builder::stream::document{}
            << "StockID" << getCurrentStockItem()->getStockID()
            << "ClientPrice" << price
            << "UpdatedProfitMargin" << profMargin
            << bsoncxx::builder::stream::finalize;
        
        auto update = bsoncxx::builder::stream::document{}
            << "$push" << bsoncxx::builder::stream::open_document
            << "clientStockArray" << doc_to_push.view()
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform the update
        dbManager.getCollection("Clients")->update_one(result->view(), update.view());
    }
    catch(mongocxx::exception e){
        std::cout << "Mongo error in UpdateClient" << e.what() << std::endl;
        return false;
    }
    catch (bsoncxx::exception e) {
        std::cout << "BSon error in UpdateClient" << e.what() << std::endl;
        return false;
    }
    return false;
}

float ClientStockItem::recalculateProfitMargin(float originalValue, float modifiedValue, float originalProfitMargin) {
    float profit = originalValue * originalProfitMargin;
    float newMargin = profit / modifiedValue;
    return newMargin;
}

void ClientStockItem::setPrice(float price) {
    clientPrice = price;
}

float ClientStockItem::getPrice() {
    return clientPrice;
}

bool ClientStockItem::updateAndAddClientStock() {
    float input = 0.0f;
    std::cout << "Please enter the clients updated price: ";
    std::cin >> input;
    setPrice(input);
    return updateClientDB(getPrice(), 
        recalculateProfitMargin(StockManager::getCurrentStockItem()->getStdPrice(), 
            getPrice(), StockManager::getCurrentStockItem()->getProfitMargin()));
}
