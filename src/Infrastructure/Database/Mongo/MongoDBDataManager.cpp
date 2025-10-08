#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>
#include "Domain/Accounts/User.h"
#include <QDebug>
bsoncxx::document::value MongoDBDataManager::buildNewUser(const std::shared_ptr<User>& newUser) {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    return document{} << "UserID" << newUser->getMongoUserID() << "UserEmail" << newUser->getEmail() << "UserPassword"
                      << newUser->getPassword() << "FirstName" << newUser->getFirstName() << "LastName"
                      << newUser->getLastName() << "AccountSetupNeeded" << true << finalize;
}

bool MongoDBDataManager::insertDocument(const std::string& collectionName, const bsoncxx::document::view& docView) {
    auto collection = InvokeDB[collectionName];

    try {
        auto result = collection.insert_one(docView);
        return result && result->result().inserted_count() == 1;
    } catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

std::optional<bsoncxx::document::value> MongoDBDataManager::findOne(const std::string& collectionName,
                                                                    const bsoncxx::document::view_or_value& filter) {
    auto collection = InvokeDB[collectionName];
    try {
        if (auto result = collection.find_one(filter)) {
            return bsoncxx::document::value{result->view()};
        } else {
            return std::nullopt;
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<bsoncxx::document::element>
MongoDBDataManager::findElement(const std::string& collectionName,
                                std::optional<bsoncxx::document::view_or_value> documentName,
                                const std::string& elementName) {
    try {
        auto& view = documentName->view();
        auto idElem = view[elementName];

        if (idElem && idElem.type() != bsoncxx::type::k_null) {
            return bsoncxx::document::element{idElem};
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<std::string> MongoDBDataManager::fetchStoredPassword(const std::string& email) {
    try {
        constexpr char kCollectionName[] = "Users";
        auto collection = InvokeDB[kCollectionName];
        auto builder = bsoncxx::builder::stream::document{};
        auto result = collection.find_one(make_document(kvp("UserEmail", email)));

        if (!result) {
            qDebug() << "[fetchStoredPassword] No user document found for email:" << email.c_str();
            return std::nullopt;
        }
        bsoncxx::document::view view = result->view();
        auto pw = view["UserPassword"];

        if (!pw) {
            qDebug() << "[fetchStoredPassword] 'UserPassword' field missing for email:" << email.c_str();
            return std::nullopt;
        }

        std::string hashedPW{pw.get_string().value};
        qDebug() << "[fetchStoredPassword] Password hash retrieved for email:" << email.c_str()
                 << "| hash length:" << static_cast<int>(hashedPW.size());
        return hashedPW;
    } catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        qDebug() << "[fetchStoredPassword] MongoDB exception for email:" << email.c_str() << "| what():" << e.what();
        return std::nullopt;
    }
}

bool MongoDBDataManager::valid_password(const std::string& password, const std::string& email) {
    if (!fetchStoredPassword(email).has_value() || !bcrypt::validatePassword(password, *fetchStoredPassword(email))) {
        return false;
    }
    return true;
}

QJsonDocument MongoDBDataManager::get_account_businesses(const std::string& user_id) {
    constexpr char kCollectionName[] = "Users";
    auto collection = InvokeDB[kCollectionName];
    auto result = collection.find_one(make_document(kvp("UserID", user_id)));

    if (result) {
        bsoncxx::document::view view = result->view();
        auto businesses = view["Businesses"];
        if (businesses && businesses.type() == bsoncxx::type::k_array) {
            std::string json_str = bsoncxx::to_json(businesses.get_array().value);
            QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(json_str));
            return jsonDoc;
        }
    }
    return QJsonDocument();
}

void MongoDBDataManager::updateDoc(const std::string& collectionName,
                                   std::optional<bsoncxx::document::value> filterDoc,
                                   std::optional<bsoncxx::document::value> replacementDoc) {
    auto result = InvokeDB[collectionName].update_one(filterDoc->view(), replacementDoc->view());
}

// std::unordered_map<int, std::string> MongoDBDataManager::listOfAll(std::string collectionName, std::string
// searchParameter) {
//     try {
//
//     }
//     catch (bsoncxx::exception e) {
//         std::cerr << "BSoncxx error: " << e.what() << std::endl;
//         return std::unordered_map<int, std::string>();
//     }
//
// }

std::optional<mongocxx::collection> MongoDBDataManager::getCollection(const std::string& collectionName) {
    try {
        return InvokeDB[collectionName];
    } catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "collection probably does not exist" << std::endl;
        return std::nullopt;
    }
}
