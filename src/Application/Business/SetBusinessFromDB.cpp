#include "Application/Business/SetBusinessFromDB.h"
#include "Application/Accounts/AccountManager.h"
std::shared_ptr<BusinessRepository> SetBusiness::setUpBusiness(const std::string businessID) {
    MongoDBDataManager dbManager;
    auto result = dbManager.findOne("Business", make_document(kvp("BusinessID", businessID)));
    if (result) {
        auto view = result->view();
        auto business = std::make_shared<BusinessRepository>(AccountManager::currentUser);

        std::unordered_set<std::string> clients;
        std::vector<std::string> stock;

        // skip businessID and UserID array
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

        std::string bizID{bizV.get_string().value};
        std::string abn{abnV.get_string().value};
        std::string phone{phoneV.get_string().value};
        std::string name{nameV.get_string().value};
        std::string address{addressV.get_string().value};
        std::string acn{acnV.get_string().value};

        for (auto& val : stockV.get_array().value) {
            stock.push_back(static_cast<std::string>(val.get_string().value));
        }
        for (auto& val : clientV.get_array().value) {
            clients.insert(static_cast<std::string>(val.get_string().value));
        }
        business->setBizID(bizID);
        business->setAbn(abn);
        business->setPhone(phone);
        business->setBizName(name);
        business->setAcn(acn);
        business->setAddress(address);
        const auto website = view["Website"];
        if (website && website.type() == bsoncxx::type::k_string)
            business->set_website_url(std::string(website.get_string().value));
        const auto logo_path = view["LogoPath"];
        if (logo_path && logo_path.type() == bsoncxx::type::k_string)
            business->set_business_logo_path(std::string(logo_path.get_string().value));
        const auto logo_data = view["LogoData"];
        if (logo_data && logo_data.type() == bsoncxx::type::k_string)
            business->set_business_logo_data(std::string(logo_data.get_string().value));
        business->setClients(clients);
        business->setStock(stock);

        return business;
    }
    std::cerr << "No business found" << std::endl;
    return nullptr;
}
