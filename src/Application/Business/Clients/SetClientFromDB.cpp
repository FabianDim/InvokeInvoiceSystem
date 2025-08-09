#include "Application/Business/Clients/SetClientFromDB.h"

std::shared_ptr<Client> SetClient::setClientFromDB(const std::string& clientID) {
    MongoDBDataManager dbManager;
    auto result = dbManager.findOne("Clients", make_document(kvp("ClientID", clientID)));

    if (result) {
        auto view = result->view();

        auto idView = view["ClientID"];
        auto nameView = view["ClientName"];
        auto phoneView = view["Phone"];
        auto emailView = view["Email"];
        auto addressView = view["Address"];
        auto stockIDsView = view["ClientStockIDs"];

        std::string id{ idView.get_utf8().value };
        std::string name{ nameView.get_utf8().value };
        std::string phone{ phoneView.get_utf8().value };
        std::string email{ emailView.get_utf8().value };
        std::string address{ addressView.get_utf8().value };

        std::vector<std::string> clientStockIDs;
        for (auto& val : stockIDsView.get_array().value) {
            clientStockIDs.push_back(static_cast<std::string>(val.get_utf8().value));
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
