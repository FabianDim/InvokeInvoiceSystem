#include "Application/Business/Clients/SetClientFromDB.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"

std::shared_ptr<Client> SetClient::setClientFromDB(const std::string& clientID) {
    MongoDBDataManager dbManager;
    auto result = dbManager.findById("Clients", clientID);

    if (result) {
        auto view = result->view();

        auto idView = view["_id"];
        auto nameView = view["ClientName"];
        auto phoneView = view["Phone"];
        auto emailView = view["Email"];
        auto addressView = view["Address"];
        auto stockIDsView = view["ClientStockIDs"];

        std::string id{ idView.get_oid().value.to_string() };
        std::string name{ nameView.get_string().value };
        std::string phone{ phoneView.get_string().value };
        std::string email{ emailView.get_string().value };
        std::string address{ addressView.get_string().value };

        std::vector<std::string> clientStockIDs;
        if (stockIDsView && stockIDsView.type() == bsoncxx::type::k_array)
        for (auto& val : stockIDsView.get_array().value) {
            clientStockIDs.push_back(val.get_oid().value.to_string());
        }

        auto client = std::make_shared<Client>();
        client->setClientID(id);
        client->setName(name);
        client->setPhoneNumber(phone);
        client->setEmail(email);
        client->setAddress(address);
        client->setClientStockIDs(clientStockIDs);

        return client;
    }

    std::cerr << "No client found with ID: " << clientID << std::endl;
    return nullptr;
}
