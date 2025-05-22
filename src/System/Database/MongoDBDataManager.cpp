#include "MongoDBDataManager.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>

bsoncxx::document::value MongoDBDataManager::buildNewUser(const std::shared_ptr<User>& newUser) {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    return document{}
    << "UserID" << newUser->getMongoUserID()
    << "UserEmail" << newUser->getEmail()
    << "UserPassword" << newUser->getPassword()
    << "FirstName" << newUser->getFirstName()
    << "LastName" << newUser->getLastName()
    << "AccountSetupNeeded" << true
    << finalize;
}

bool MongoDBDataManager::insertDocument(const std::string& collectionName, const bsoncxx::document::view& docView) {
    auto collection = InvokeDB[collectionName];

    try {
        auto result = collection.insert_one(docView);
        return result && result->result().inserted_count() == 1;
    }
    catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

std::optional<bsoncxx::document::value> MongoDBDataManager::findOne(const std::string& collectionName, const bsoncxx::document::view_or_value& filter) {
    auto collection = InvokeDB[collectionName];
    try {
        if (auto result = collection.find_one(filter)) {
            return bsoncxx::document::value{ result->view() };
        }
        else {
            return std::nullopt;
        }
    }
    catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<bsoncxx::document::element> MongoDBDataManager::findElement(const std::string& collectionName, std::optional<bsoncxx::document::view_or_value> documentName, const std::string& elementName) {
    try {
        auto& view = documentName->view();
        auto idElem = view[elementName];

        if (idElem && idElem.type() != bsoncxx::type::k_null) {
            return bsoncxx::document::element{ idElem };
        }
    }
    catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<std::string> MongoDBDataManager::fetchStoredPassword(const std::string& email) {
    constexpr char kCollectionName[] = "Users";
    auto collection = InvokeDB[kCollectionName];
    auto builder = bsoncxx::builder::stream::document{};
    auto result = collection.find_one(make_document(kvp("UserEmail", email)));

    if (!result) {
        return std::nullopt;
    }

    bsoncxx::document::view view = result->view();
    auto pw = view["UserPassword"];

    if (!pw) {
        return std::nullopt;
    }

    std::string hashedPW{ pw.get_utf8().value };
    return hashedPW;
}

bool MongoDBDataManager::validPassword(const std::string& password, const std::string& email) {
    if (!fetchStoredPassword(email) || !bcrypt::validatePassword(password, *fetchStoredPassword(email))) {
        return false;
    }
    return true;
}

bool MongoDBDataManager::makeDBArray(const std::string& collectionName, streamDocument doc, int nbItems) {
    return false;
}

void MongoDBDataManager::updateDoc(const std::string& collectionName, std::optional<streamDocument> filterDoc, std::optional<bsoncxx::document::value> replacementDoc) {
    auto result = InvokeDB[collectionName].update_one(filterDoc->view(), replacementDoc->view());
}
