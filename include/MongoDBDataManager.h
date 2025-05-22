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

using streamDocument = bsoncxx::builder::stream::document;

class MongoDBDataManager {
    friend class AccountManager;
    friend class SetUser;
    friend class BusinessDetails;
    MongoDBHandler dbHandler;
    const mongocxx::database InvokeDB;

public:
    MongoDBDataManager()
        : InvokeDB{ dbHandler.getDatabase() } {
    }

    bool insertDocument(const std::string& collectionName, const bsoncxx::document::view& docView);
    bsoncxx::document::value buildNewUser(const std::shared_ptr<User>& newUser);
    void updateElement(const std::string& collectionName, const std::string& docparameter, const std::string& email, const std::string& prevVal, const bool updateValue);

private:
    std::optional<bsoncxx::document::value> findOne(const std::string& collectionName, const bsoncxx::document::view_or_value& filter);
    //std::string getUserOID(const std::string& collectionName, std::optional<bsoncxx::document::view_or_value>& view);
    std::optional<bsoncxx::document::element> findElement(const std::string& collectionName, std::optional<bsoncxx::document::view_or_value> documentName, const std::string& elementName);
    std::optional<std::string> fetchStoredPassword(const std::string& email);
    bool validPassword(const std::string& password, const std::string& hashedPW);
    bool makeDBArray(const std::string& collectionName, streamDocument doc, int nbItems);
    void updateDoc(const std::string& collectionName, std::optional<streamDocument> filterDoc, std::optional<bsoncxx::document::value> replacementDoc);
};
