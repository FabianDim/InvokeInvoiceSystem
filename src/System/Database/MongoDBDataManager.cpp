#include "MongoDBDataManager.h"


bool MongoDBDataManager::writeNewUser(std::shared_ptr<User> newUser) {
    constexpr char kCollectionName[] = "Users";
    auto collection = InvokeDB[kCollectionName];

    bsoncxx::builder::basic::document builder;
    builder.append(
        kvp("UserEmail", newUser->getEmail()),
        kvp("UserPassword", newUser->getPassword()),
        kvp("FirstName", newUser->getFirstName())
    );
    auto doc = builder.extract();

    collection.insert_one(doc.view());
    return true;
}
bool MongoDBDataManager::updateUser(std::shared_ptr<User> oldUser) {
	return false;
}

bool MongoDBDataManager::removeUser(std::shared_ptr<User> user) {
	return false;
}
