#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>
#include "Domain/Accounts/User.h"
#include <QDebug>
#include "Application/Business/BusinessRepository.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include "Application/Business/SetBusinessFromDB.h"
#include "Infrastructure/Security/PasswordHashing/bcrypt.h"
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
    const auto stored_password = fetchStoredPassword(email);
    return stored_password.has_value() && bcrypt::validatePassword(password, *stored_password);
}

MongoDBDataManager::AccountCreationResult MongoDBDataManager::create_account(const QJsonObject& details) {
    const auto email = details.value("email").toString().trimmed().toStdString();
    const auto password = details.value("password").toString().toStdString();
    qDebug() << "[MongoDBDataManager::create_account] Starting account creation"
             << "| email:" << QString::fromStdString(email)
             << "| password length:" << static_cast<int>(password.size())
             << "| firstName:" << details.value("firstName").toString()
             << "| lastName:" << details.value("lastName").toString();

    if (email.empty() || password.empty()) {
        qWarning() << "[MongoDBDataManager::create_account] Missing required signup field"
                   << "| has email:" << !email.empty()
                   << "| has password:" << !password.empty();
        return AccountCreationResult::InvalidInput;
    }

    qDebug() << "[MongoDBDataManager::create_account] Checking for existing email"
             << "| email:" << QString::fromStdString(email);
    auto collection = InvokeDB["Users"];
    try {
        if (collection.find_one(make_document(kvp("UserEmail", email)))) {
            qWarning() << "[MongoDBDataManager::create_account] Email already exists"
                       << "| email:" << QString::fromStdString(email);
            return AccountCreationResult::EmailExists;
        }
    } catch (const mongocxx::exception& e) {
        qWarning() << "[MongoDBDataManager::create_account] MongoDB exception during email check"
                   << "| email:" << QString::fromStdString(email)
                   << "| what():" << e.what();
        return AccountCreationResult::DatabaseError;
    }

    const auto user_id = std::string("USR") +
                         QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper().toStdString();
    qDebug() << "[MongoDBDataManager::create_account] Generated user id"
             << "| email:" << QString::fromStdString(email)
             << "| user id:" << QString::fromStdString(user_id);
    const auto document = bsoncxx::builder::stream::document{}
        << "UserID" << user_id
        << "UserEmail" << email
        << "UserPassword" << bcrypt::generateHash(password)
        << "FirstName" << details.value("firstName").toString().trimmed().toStdString()
        << "LastName" << details.value("lastName").toString().trimmed().toStdString()
        << "AccountSetupNeeded" << true
        << bsoncxx::builder::stream::finalize;
    qDebug() << "[MongoDBDataManager::create_account] Inserting user document"
             << "| email:" << QString::fromStdString(email)
             << "| user id:" << QString::fromStdString(user_id);
    try {
        const auto result = collection.insert_one(document.view());
        if (result && result->result().inserted_count() == 1) {
            qDebug() << "[MongoDBDataManager::create_account] User document inserted"
                     << "| email:" << QString::fromStdString(email)
                     << "| user id:" << QString::fromStdString(user_id);
            return AccountCreationResult::Created;
        }
        qWarning() << "[MongoDBDataManager::create_account] User document insert returned no inserted document"
                   << "| email:" << QString::fromStdString(email)
                   << "| user id:" << QString::fromStdString(user_id);
    } catch (const mongocxx::exception& e) {
        qWarning() << "[MongoDBDataManager::create_account] MongoDB exception during user insert"
                   << "| email:" << QString::fromStdString(email)
                   << "| user id:" << QString::fromStdString(user_id)
                   << "| what():" << e.what();
        return AccountCreationResult::DatabaseError;
    }

    return AccountCreationResult::DatabaseError;
}

QJsonDocument MongoDBDataManager::get_account_businesses(const std::string& user_id) {
    try {
        constexpr char kCollectionName[] = "Users";
        auto collection = InvokeDB[kCollectionName];
        qDebug() << "Fetching businesses for user ID:" << QString::fromStdString(user_id);
        auto result = collection.find_one(make_document(kvp("UserID", user_id)));

        if (result) {
            bsoncxx::document::view view = result->view();
            auto businesses = view["BusinessIDs"];

            QJsonObject json_id;

            if (businesses && businesses.type() == bsoncxx::type::k_array) {
                SetBusiness set_business;
                for (auto& elem : businesses.get_array().value) {
                    auto item = set_business.setUpBusiness(std::string(elem.get_string().value));
                    if (item) {
                        QJsonObject json_object;
                        json_object["BusinessID"] = QString::fromStdString(item->getBizID());
                        json_object["BusinessName"] = QString::fromStdString(item->getBizName());
                        json_object["Address"] = QString::fromStdString(item->getAddress());
                        json_object["ABN"] = QString::fromStdString(item->getAbn());
                        json_object["ACN"] = QString::fromStdString(item->getAcn());
                        json_object["Phone"] = QString::fromStdString(item->getPhone());
                        json_id[QString::fromStdString(std::string(elem.get_string().value))] = json_object;
                    }
                }
                return QJsonDocument(json_id);
            }
        }
        return QJsonDocument();
    } catch (mongocxx::exception e) {
        qDebug() << "Error processing business IDs for user ID:" << QString::fromStdString(user_id);
        qDebug() << "MongoDB exception:" << e.what();
        return QJsonDocument();
    }
}

QJsonDocument MongoDBDataManager::list_resources(const std::string& resource, const std::string& user_id) {
    QJsonArray result;
    const auto user = findOne("Users", make_document(kvp("UserID", user_id)));
    if (!user)
        return QJsonDocument(result);

    const auto business_ids = user->view()["BusinessIDs"];
    if (resource == "businesses") {
        if (business_ids && business_ids.type() == bsoncxx::type::k_array) {
            for (const auto& id : business_ids.get_array().value) {
                if (id.type() != bsoncxx::type::k_string)
                    continue;
                const auto business = findOne("Business", make_document(kvp("BusinessID", id.get_string().value)));
                if (business) {
                    const auto view = business->view();
                    QJsonObject item{
                        {"BusinessID", QString::fromUtf8(view["BusinessID"].get_string().value)},
                        {"BusinessName", QString::fromUtf8(view["BusinessName"].get_string().value)},
                        {"ABN", QString::fromUtf8(view["ABN"].get_string().value)},
                        {"Phone", QString::fromUtf8(view["Phone"].get_string().value)},
                        {"BusinessAddress", QString::fromUtf8(view["BusinessAddress"].get_string().value)}};
                    result.append(item);
                }
            }
        }
        return QJsonDocument(result);
    }

    const char* collection = resource == "clients" ? "Clients" : "Stock";
    const char* array_name = resource == "clients" ? "ClientIDs" : "StockIDs";
    if (!business_ids || business_ids.type() != bsoncxx::type::k_array)
        return QJsonDocument(result);
    for (const auto& business_id : business_ids.get_array().value) {
        if (business_id.type() != bsoncxx::type::k_string)
            continue;
        const auto business = findOne("Business", make_document(kvp("BusinessID", business_id.get_string().value)));
        if (!business)
            continue;
        const auto ids = business->view()[array_name];
        if (!ids || ids.type() != bsoncxx::type::k_array)
            continue;
        for (const auto& id : ids.get_array().value) {
            if (id.type() != bsoncxx::type::k_string)
                continue;
            const std::string id_key = resource == "clients" ? "ClientID" : "StockID";
            const auto item = findOne(collection, make_document(kvp(id_key, id.get_string().value)));
            if (!item)
                continue;
            const auto view = item->view();
            QJsonObject json{
                {resource == "clients" ? "ClientID" : "StockID", QString::fromUtf8(id.get_string().value)},
                {"Name", QString::fromUtf8(view[resource == "clients" ? "ClientName" : "Name"].get_string().value)},
                {"BusinessID", QString::fromUtf8(business_id.get_string().value)}};
            if (resource == "clients") {
                json["Phone"] = QString::fromUtf8(view["Phone"].get_string().value);
                json["Email"] = QString::fromUtf8(view["Email"].get_string().value);
            } else {
                json["Price"] = view["StdPrice"].get_double().value;
                json["Quantity"] = view["Quantity"].get_int32().value;
                json["Unit"] = QString::fromUtf8(view["Unit Type"].get_string().value);
            }
            result.append(json);
        }
    }
    return QJsonDocument(result);
}

bool MongoDBDataManager::save_resource(const std::string& resource, QJsonObject data, const std::string& user_id) {
    try {
        const auto id = QString::fromStdString(resource == "business" ? "BUS"
                                               : resource == "client" ? "CLI"
                                                                      : "STK") +
                        QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper();
        if (resource == "business") {
            const auto doc = make_document(kvp("BusinessID", id.toStdString()),
                                           kvp("UserID", user_id),
                                           kvp("ABN", data.value("ABN").toString().toStdString()),
                                           kvp("Phone", data.value("Business phone").toString().toStdString()),
                                           kvp("BusinessName", data.value("Business name").toString().toStdString()),
                                           kvp("BusinessAddress", data.value("Address").toString().toStdString()),
                                           kvp("ACN", data.value("ACN").toString().toStdString()),
                                           kvp("ClientIDs", bsoncxx::builder::basic::array{}),
                                           kvp("StockIDs", bsoncxx::builder::basic::array{}));
            if (!insertDocument("Business", doc.view()))
                return false;
            const auto user = findOne("Users", make_document(kvp("UserID", user_id)));
            if (!user)
                return false;
            const auto update = make_document(kvp("$addToSet", make_document(kvp("BusinessIDs", id.toStdString()))));
            return static_cast<bool>(InvokeDB["Users"].update_one(user->view(), update.view()));
        }

        const auto business_id = data.value("BusinessID").toString().toStdString();
        if (business_id.empty())
            return false;
        bsoncxx::builder::basic::document doc;
        if (resource == "client") {
            doc.append(kvp("ClientID", id.toStdString()));
            doc.append(kvp("BusinessID", business_id));
            doc.append(kvp("ClientName", data.value("Name").toString().toStdString()));
            doc.append(kvp("Phone", data.value("Phone").toString().toStdString()));
            doc.append(kvp("Email", data.value("Email").toString().toStdString()));
            doc.append(kvp("Address", data.value("Address").toString().toStdString()));
        } else {
            doc.append(kvp("StockID", id.toStdString()));
            doc.append(kvp("BusinessID", business_id));
            doc.append(kvp("Name", data.value("Name").toString().toStdString()));
            doc.append(kvp("Quantity", data.value("Quantity").toInt()));
            doc.append(kvp("StdPrice", data.value("Price").toDouble()));
            doc.append(kvp("ProfitMargin", data.value("Margin").toDouble()));
            doc.append(kvp("Unit Type", data.value("Unit").toString().toStdString()));
            bsoncxx::builder::basic::array keywords;
            for (const auto& keyword : data.value("Keywords").toString().split(',', Qt::SkipEmptyParts))
                keywords.append(keyword.trimmed().toLower().toStdString());
            doc.append(kvp("ProductKeywords", keywords));
        }
        if (!insertDocument(resource == "client" ? "Clients" : "Stock", doc.view()))
            return false;
        const std::string key = resource == "client" ? "ClientIDs" : "StockIDs";
        const auto value = id.toStdString();
        const auto update = make_document(kvp("$addToSet", make_document(kvp(key, value))));
        const auto business = findOne("Business", make_document(kvp("BusinessID", business_id)));
        return business && static_cast<bool>(InvokeDB["Business"].update_one(business->view(), update.view()));
    } catch (const std::exception& error) {
        qWarning() << "Could not save resource:" << error.what();
        return false;
    }
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
