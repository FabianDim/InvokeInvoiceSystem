#pragma once  
#include "pch.h"  
#include <mongocxx/client.hpp>  
#include <mongocxx/instance.hpp>  
#include <mongocxx/uri.hpp>  
#include <bsoncxx/builder/stream/document.hpp>  
#include "bsoncxx/builder/basic/kvp.hpp"  
#include "Accounts/User.h"
#include "MongoDBHandler.h"
#include "Accounts/PasswordHashing/bcrypt.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

using bsoncxx::builder::basic::array;

class MongoDBDataManager {
    friend class AccountManager;
    friend class SetUser;
    friend class BusinessDetails;
    friend class BusinessRepository;
    friend class InvoiceMenu;
    friend class SetBusiness;
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
    array makeDBArray(const bsoncxx::document::view_or_value& initDoc);
    void updateDoc(const std::string& collectionName, std::optional<bsoncxx::v_noabi::document::value> filterDoc, std::optional<bsoncxx::document::value> replacementDoc);
    std::optional<mongocxx::collection> getCollection(const std::string& collectionName);
};
