#pragma once  
#include "pch.h"  
#include <mongocxx/client.hpp>  
#include <mongocxx/instance.hpp>  
#include <mongocxx/uri.hpp>  
#include <bsoncxx/builder/stream/document.hpp>  
#include <bsoncxx/builder/basic/kvp.hpp>  
#include <User.h>
#include <MongoDBHandler.h>
#include "bcrypt.h"

using bsoncxx::builder::basic::kvp;  
using bsoncxx::builder::basic::make_document;  

class MongoDBDataManager {
    MongoDBHandler dbHandler;
    const mongocxx::database InvokeDB; // Add this member variable

public:
    MongoDBDataManager()
        : InvokeDB{ dbHandler.getDatabase() } { // Initialize it in the constructor
    }

    bool writeNewUser(const std::shared_ptr<User>& newUser);
    optional<string> fetchStoredPassword(const std::string& email);
    bool updateUser(std::shared_ptr<User> oldUser);
    bool removeUser(std::shared_ptr<User> user);
};
