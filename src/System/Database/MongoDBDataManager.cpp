#include "MongoDBDataManager.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>

bsoncxx::document::value  MongoDBDataManager::buildNewUser(const std::shared_ptr<User>& newUser) {
        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::finalize;

        // build the document with the stream API
        return document{} 
            << "UserEmail" << newUser->getEmail()
            << "UserPassword" << newUser->getPassword()
            << "FirstName" << newUser->getFirstName()
            << "LastName" << newUser->getLastName()
            << finalize;
}
bool MongoDBDataManager::insertDocument(const string& collectionName, const bsoncxx::document::view& docView) {
    auto collection = InvokeDB[collectionName];

    try {
        auto result = collection.insert_one(docView);
        return result && result->result().inserted_count() == 1;
    }
    catch (const mongocxx::exception& e) {
        cerr << e.what() << endl;
        return false;
    }
}

//finds a document view in the database.
optional<bsoncxx::document::value> MongoDBDataManager::findOne(const string& collectionName, const bsoncxx::document::view_or_value& filter) {
    auto collection = InvokeDB[collectionName];
    try {
        if(auto result = collection.find_one(filter)){
            return bsoncxx::document::value{ result->view() };
        }
        else {
            return std::nullopt;
        }
    }
    catch (const mongocxx::exception& e) {
        cerr << e.what() << endl;
        return nullopt;
    }
}

string MongoDBDataManager::getUserOID(const string& collectionName, optional<bsoncxx::document::view_or_value>& viewOpt) {

    auto docOpt = findOne(collectionName, *viewOpt);
    if (!docOpt) {
        // not found (or error)
        return {};
    }
    auto view = docOpt->view();
    auto idElem = view["_id"];
    if (!idElem || idElem.type() != bsoncxx::type::k_oid) {
        // no _id, or wrong type
        return {};
    }
    bsoncxx::oid oid = idElem.get_oid().value;
    return oid.to_string();
}



//break up the functions into smaller less specific functions to create more flexibility
optional<string> MongoDBDataManager::fetchStoredPassword(const std::string& email) {
    constexpr char kCollectionName[] = "Users";
    auto collection = InvokeDB[kCollectionName];
    auto builder = bsoncxx::builder::stream::document{};
    auto result = collection.find_one(make_document(kvp("UserEmail", email)));

    if (!result) {
        return nullopt;
    }

    bsoncxx::document::view view = result->view();

    auto pw = view["UserPassword"];

    if (!pw) {
        return nullopt;;
    }
    auto passString = pw.get_utf8().value;
    string hashedPW{ pw.get_utf8().value };
    return hashedPW;
}

bool MongoDBDataManager::validPassword(const string& password, const string& email) {
    if(!fetchStoredPassword(email) or !bcrypt::validatePassword(password, *fetchStoredPassword(email))) {
        return false;
    }
    return true;
}

