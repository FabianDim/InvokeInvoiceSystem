#include "Accounts/SetUserFromDB.h"
#include <bsoncxx/document/view_or_value.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>

std::shared_ptr<User> SetUser::setUserOnLogin(const std::string& email, const std::string& password) {
    MongoDBDataManager mongoDataManager;
    auto filter = bsoncxx::builder::stream::document{}
    << "UserEmail" << email << bsoncxx::builder::stream::finalize;
    auto result = mongoDataManager.findOne("Users", filter.view());

    if (result) {
        auto user = std::make_shared<User>(email, password);
        auto view = result->view();
        auto id = view["UserID"];
        auto first = view["FirstName"];
        auto last = view["LastName"];
        auto email = view["UserEmail"];
        auto pass = view["UserPassword"];
        if (!first || first.type() != bsoncxx::type::k_utf8) {
            throw std::runtime_error("FirstName missing or wrong type");
        }
        std::string userID{ id.get_utf8().value };
        std::string firstName{ first.get_utf8().value };
        std::string lastName{ last.get_utf8().value };
        std::string userEmail{ email.get_utf8().value };
        std::string password{ pass.get_utf8().value };
        user->setMongoUserID(userID);
        user->setFirstName(firstName);
        user->setLastName(lastName);
        user->setUserEmail(userEmail);
        user->setPassword(password);
        return user;
    }

    return nullptr;
}
