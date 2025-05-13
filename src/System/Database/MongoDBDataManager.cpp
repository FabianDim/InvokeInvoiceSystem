#include "MongoDBDataManager.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>

bool MongoDBDataManager::writeNewUser(const std::shared_ptr<User>& newUser) {
        constexpr char kCollectionName[] = "Users";
        auto collection = InvokeDB[kCollectionName];

		

        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::finalize;

        // build the document with the stream API
        auto doc = document{} 
            << "UserEmail" << newUser->getEmail()
            << "UserPassword" << newUser->getPassword()
            << "FirstName" << newUser->getFirstName()
            << "LastName" << newUser->getLastName()
            << finalize;

        // insert into MongoDB
        auto result = collection.insert_one(doc.view());

        // Check if insert succeeded
        return result && result->result().inserted_count() == 1;
    
}

bool MongoDBDataManager::findUser(const std::string& email, const std::string& password) {
	constexpr char kCollectionName[] = "Users";
    auto collection = InvokeDB[kCollectionName];

    auto result = collection.find_one(make_document(kvp("UserEmail", email)));

    if (!result) {
        std::cout << "No user with that email found!" << std::endl;
    }
    //else if (password == ) {

    //}


    return false;
}


bool MongoDBDataManager::updateUser(std::shared_ptr<User> oldUser) {
	return false;
}

bool MongoDBDataManager::removeUser(std::shared_ptr<User> user) {
	return false;
}
