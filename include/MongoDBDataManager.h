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
    friend class  AccountManager;
    MongoDBHandler dbHandler;
    const mongocxx::database InvokeDB; // Add this member variable
    
public:
    MongoDBDataManager()
        : InvokeDB{ dbHandler.getDatabase() } { // Initialize it in the constructor
    }

    bsoncxx::document::value  buildNewUser(const std::shared_ptr<User>& newUser);
private:
    bool insertDocument(const string& collectionName, const bsoncxx::document::view& docView);
    optional<bsoncxx::document::view> findOne(const string& collectionName, const bsoncxx::document::view_or_value& filter);
    //optional<bsoncxx::document::element> findElement(const string& collectionName, const string& documentName, const string& elementName);
    optional<string> fetchStoredPassword(const std::string& email);
    bool validPassword(const string& password, const string& hashedPW);

};
