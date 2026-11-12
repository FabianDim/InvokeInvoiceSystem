#include "Infrastructure/Logging/Logging.h"
#include "Application/Accounts/SetUserFromDB.h"
#include <bsoncxx/document/view_or_value.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>

std::shared_ptr<User> SetUser::setUserOnLogin(const std::string& email, const std::string& password) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::database(), "SetUser.setUserOnLogin");

    auto filter = bsoncxx::builder::stream::document{} << "UserEmail" << email << bsoncxx::builder::stream::finalize;
    auto result = mongoDataManager.findOne("Users", filter.view());

    if (result) {
        auto user = std::make_shared<User>(email, password);
        auto view = result->view();
        auto id = view["_id"];
        auto first = view["FirstName"];
        auto last = view["LastName"];
        auto email = view["UserEmail"];
        auto pass = view["UserPassword"];
        auto business_id_array = view["BusinessIDs"];
        if (!first || first.type() != bsoncxx::type::k_string) {
            throw std::runtime_error("FirstName missing or wrong type");
        }
        std::string userID{id.get_oid().value.to_string()};
        std::string firstName{first.get_string().value};
        std::string lastName{last.get_string().value};
        std::string userEmail{email.get_string().value};
        std::string password{pass.get_string().value};
        std::vector<std::string> business_ids;
        if (business_id_array && business_id_array.type() == bsoncxx::type::k_array) {
            for (const auto& business_id : business_id_array.get_array().value) {
                if (business_id.type() != bsoncxx::type::k_oid) {
                    throw std::runtime_error("BusinessIDs contains a non-ObjectId value");
                }
                business_ids.emplace_back(business_id.get_oid().value.to_string());
            }
        }
        user->set_user_business(business_ids);
        user->setMongoUserID(userID);
        user->setFirstName(firstName);
        user->setLastName(lastName);
        user->setUserEmail(userEmail);
        user->setPassword(password);
        return user;
    }

    return nullptr;
}

bool SetUser::addBusinessToUser(const std::string& userID, const std::string& businessID) {
    const auto user = Infrastructure::Database::parse_object_id(userID);
    const auto business = Infrastructure::Database::parse_object_id(businessID);
    if (!user || !business)
        return false;
    const auto owned = mongoDataManager.findOne("Business", make_document(kvp("_id", *business), kvp("UserID", *user)));
    if (!owned)
        return false;
    const auto result = mongoDataManager.getCollection("Users")->update_one(
        make_document(kvp("_id", *user)),
        make_document(kvp("$addToSet", make_document(kvp("BusinessIDs", *business)))));
    return result && result->matched_count() == 1;
}
