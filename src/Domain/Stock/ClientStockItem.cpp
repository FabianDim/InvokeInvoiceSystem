#include "Domain/Stock/ClientStockItem.h"
#include "Application/Stock/StockManager.h"
#include "bsoncxx/exception/exception.hpp"
#include "bsoncxx/builder/stream/document.hpp"
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
        std::cerr << "Mongo error in UpdateClient" << e.what() << std::endl;
        return false;
    }
    catch (bsoncxx::exception e) {
        std::cerr << "BSon error in UpdateClient" << e.what() << std::endl;
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
    auto cliMap = ClientManager::fetchBizClients();
    std::unordered_map<int, std::string> choiceMap;
    int count = 1;
    for (auto& client : cliMap) {
        std::cout << count << ": " << client.second << " | " << client.first << std::endl;
        choiceMap[count] = client.first;
        count++;
    }
    int choice;
    do {
        std::cout << "Choose the client number of the client you'd like to update: ";
        std::cin >> choice;
        if (choiceMap.contains(choice)) {
            ClientManager::setClient(SetClient::setClientFromDB(choiceMap[choice]));//need to get the client obj
            break;//setclient from db class
        }
    } while (std::to_string(choice) != "*");
    float input = 0.0f;
    std::cout << "Please enter the clients updated price: ";
    std::cin >> input;
    setPrice(input);
    return updateClientDB(getPrice(), 
        recalculateProfitMargin(StockManager::getCurrentStockItem()->getStdPrice(), 
            getPrice(), StockManager::getCurrentStockItem()->getProfitMargin()));
}
