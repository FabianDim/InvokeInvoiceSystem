#include "Infrastructure/Logging/Logging.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>
#include "Domain/Accounts/User.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include "Infrastructure/Security/PasswordHashing/bcrypt.h"
#include "Utils/BusinessLogo.h"
#include <mongocxx/client_session.hpp>
#include <mongocxx/options/find_one_and_update.hpp>
#include <QMap>
#include <cmath>

QString MongoDBDataManager::reserve_invoice_number(const std::string& user_id, const std::string& business_id,
                                                   const QString& prefix) {
    const auto owner = Infrastructure::Database::parse_object_id(user_id);
    const auto business = Infrastructure::Database::parse_object_id(business_id);
    if (!owner || !business || prefix.size() > 40)
        throw std::invalid_argument("Invalid business or invoice prefix (maximum 40 characters).");
    mongocxx::options::find_one_and_update options;
    options.return_document(mongocxx::options::return_document::k_after);
    const auto result = database()["Business"].find_one_and_update(
        make_document(kvp("_id", *business), kvp("UserID", *owner)),
        make_document(kvp("$inc", make_document(kvp("InvoiceSequence", std::int64_t{1})))), options);
    if (!result) throw std::invalid_argument("Business is not available for this account.");
    const auto value = result->view()["InvoiceSequence"];
    const auto sequence = value.type() == bsoncxx::type::k_int64 ? value.get_int64().value : value.get_int32().value;
    return prefix + QString::number(sequence).rightJustified(6, '0');
}

bool MongoDBDataManager::save_invoice_settings(const std::string& user_id, const QJsonObject& settings) {
    const auto owner = Infrastructure::Database::parse_object_id(user_id);
    const auto business = Infrastructure::Database::parse_object_id(settings.value("BusinessID").toString().toStdString());
    if (!owner || !business || !settings.value("InvoicePrefix").isString() ||
        settings.value("InvoicePrefix").toString().size() > 40) return false;
    const auto result = database()["Business"].update_one(
        make_document(kvp("_id", *business), kvp("UserID", *owner)),
        make_document(kvp("$set", make_document(kvp("InvoicePrefix", settings.value("InvoicePrefix").toString().toStdString())))));
    return result && result->matched_count() == 1;
}

void MongoDBDataManager::complete_invoice(const std::string& user_id, const std::string& business_id,
                                         const std::string& client_id, const QString& invoice_id,
                                         const QString& number, const QJsonArray& stock) {
    const auto owner = Infrastructure::Database::parse_object_id(user_id);
    const auto business = Infrastructure::Database::parse_object_id(business_id);
    const auto client = Infrastructure::Database::parse_object_id(client_id);
    if (!owner || !business || !client) throw std::invalid_argument("Invalid invoice ownership.");
    QMap<QString, int> quantities;
    for (const auto& value : stock) {
        const auto item = value.toObject();
        const auto id = item.value("_id").toString();
        if (id.isEmpty()) continue; // An ad hoc line has no inventory record.
        const auto quantity = item.value("Quantity").toDouble(-1);
        if (!Infrastructure::Database::parse_object_id(id.toStdString()) || !std::isfinite(quantity) ||
            quantity < 1 || quantity > 1'000'000 || std::floor(quantity) != quantity ||
            quantities.value(id) > 1'000'000 - quantity)
            throw std::invalid_argument("Invalid stock quantity or record ID.");
        quantities[id] += static_cast<int>(quantity);
    }
    auto db = database();
    auto session = dbHandler->client.start_session();
    const auto identity = make_document(kvp("_id", bsoncxx::oid(invoice_id.toStdString())));
    const auto payload = QJsonDocument(stock).toJson(QJsonDocument::Compact).toStdString();
    session.with_transaction([&](mongocxx::client_session* transaction) {
        if (!db["Business"].find_one(*transaction, make_document(kvp("_id", *business), kvp("UserID", *owner))) ||
            !db["Clients"].find_one(*transaction, make_document(kvp("_id", *client), kvp("BusinessID", *business))))
            throw std::invalid_argument("Invoice business or client is no longer available.");
        if (const auto existing = db["Invoices"].find_one(*transaction, identity.view())) {
            if (std::string(existing->view()["ItemsJson"].get_string().value) != payload)
                throw std::invalid_argument("This invoice is complete. Start a new invoice to change its items.");
            return;
        }
        for (auto it = quantities.constBegin(); it != quantities.constEnd(); ++it) {
            const auto result = db["Stock"].update_one(*transaction,
                make_document(kvp("_id", bsoncxx::oid(it.key().toStdString())), kvp("BusinessID", *business),
                              kvp("Quantity", make_document(kvp("$gte", it.value())))),
                make_document(kvp("$inc", make_document(kvp("Quantity", -it.value())))));
            if (!result || result->matched_count() != 1)
                throw std::invalid_argument("Insufficient stock, or an item does not belong to this business. Refresh stock and try again.");
        }
        const auto result = db["Invoices"].insert_one(*transaction, make_document(
            kvp("_id", bsoncxx::oid(invoice_id.toStdString())), kvp("UserID", *owner), kvp("BusinessID", *business),
            kvp("ClientID", *client), kvp("InvoiceNumber", number.toStdString()), kvp("ItemsJson", payload)));
        if (!result) throw std::runtime_error("Could not complete invoice.");
    });
}
bsoncxx::document::value MongoDBDataManager::buildNewUser(const std::shared_ptr<User>& newUser) {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    return document{} << "UserEmail" << newUser->getEmail() << "UserPassword" << newUser->getPassword() << "FirstName"
                      << newUser->getFirstName() << "LastName" << newUser->getLastName() << "AccountSetupNeeded" << true
                      << finalize;
}

bool MongoDBDataManager::insertDocument(const std::string& collectionName,
                                        const bsoncxx::document::view& docView,
                                        std::string* inserted_id) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.insertDocument");
    auto collection = database()[collectionName];

    try {
        auto result = collection.insert_one(docView);
        qCDebug(Infrastructure::Logging::database)
            << "event=database.insert.completed acknowledged=" << static_cast<bool>(result)
            << "inserted_count=" << (result ? result->result().inserted_count() : 0);
        if (!result || result->result().inserted_count() != 1)
            return false;
        if (inserted_id)
            *inserted_id = result->inserted_id().get_oid().value.to_string();
        return true;
    } catch (const mongocxx::exception& e) {
        qCWarning(Infrastructure::Logging::database) << "event=database.exception code=" << e.code().value();
        return false;
    }
}

std::optional<bsoncxx::document::value> MongoDBDataManager::findOne(const std::string& collectionName,
                                                                    const bsoncxx::document::view_or_value& filter) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(), "MongoDBDataManager.findOne");
    auto collection = database()[collectionName];
    try {
        if (auto result = collection.find_one(filter)) {
            qCDebug(Infrastructure::Logging::database) << "event=database.find.completed found=true";
            return bsoncxx::document::value{result->view()};
        } else {
            qCDebug(Infrastructure::Logging::database) << "event=database.find.completed found=false";
            return std::nullopt;
        }
    } catch (const mongocxx::exception& e) {
        qCWarning(Infrastructure::Logging::database) << "event=database.exception code=" << e.code().value();
        return std::nullopt;
    }
}

std::optional<bsoncxx::document::value> MongoDBDataManager::findById(const std::string& collection,
                                                                     const std::string& id) {
    const auto oid = Infrastructure::Database::parse_object_id(id);
    if (!oid)
        return std::nullopt;
    return findOne(collection, make_document(kvp("_id", *oid)));
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
        qCWarning(Infrastructure::Logging::database) << "event=database.exception code=" << e.code().value();
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<std::string> MongoDBDataManager::fetchStoredPassword(const std::string& email) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.fetchStoredPassword");
    try {
        constexpr char kCollectionName[] = "Users";
        auto collection = database()[kCollectionName];
        auto builder = bsoncxx::builder::stream::document{};
        auto result = collection.find_one(make_document(kvp("UserEmail", email)));

        if (!result) {
            qCDebug(Infrastructure::Logging::database) << "event=auth.password_lookup.missing_account";
            return std::nullopt;
        }
        bsoncxx::document::view view = result->view();
        auto pw = view["UserPassword"];

        if (!pw) {
            qCDebug(Infrastructure::Logging::database) << "event=auth.password_lookup.missing_hash";
            return std::nullopt;
        }

        std::string hashedPW{pw.get_string().value};
        qCDebug(Infrastructure::Logging::database) << "event=auth.password_lookup.completed";
        return hashedPW;
    } catch (const mongocxx::exception& e) {
        qCWarning(Infrastructure::Logging::database) << "event=database.exception code=" << e.code().value();
        qCDebug(Infrastructure::Logging::database) << "event=auth.password_lookup.failed";
        return std::nullopt;
    }
}

bool MongoDBDataManager::valid_password(const std::string& password, const std::string& email) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.valid_password");
    const auto stored_password = fetchStoredPassword(email);
    return stored_password.has_value() && bcrypt::validatePassword(password, *stored_password);
}

std::optional<std::string> MongoDBDataManager::authenticate_account(const std::string& email,
                                                                    const std::string& password) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.authenticate_account");
    if (email.empty() || password.empty()) {
        qCWarning(Infrastructure::Logging::auth) << "event=auth.credentials.rejected reason=missing_fields";
        return std::nullopt;
    }
    const auto document = database()["Users"].find_one(make_document(kvp("UserEmail", email)));
    if (!document) {
        qCWarning(Infrastructure::Logging::auth) << "event=auth.credentials.rejected reason=account_not_found";
        return std::nullopt;
    }
    const auto view = document->view();
    const auto hash = view["UserPassword"];
    const auto id = view["_id"];
    if (!hash || hash.type() != bsoncxx::type::k_string || !id || id.type() != bsoncxx::type::k_oid) {
        qCWarning(Infrastructure::Logging::auth) << "event=auth.credentials.rejected reason=invalid_account_record";
        return std::nullopt;
    }
    qCDebug(Infrastructure::Logging::auth) << "event=auth.password_check.started";
    if (!bcrypt::validatePassword(password, std::string(hash.get_string().value))) {
        qCWarning(Infrastructure::Logging::auth) << "event=auth.credentials.rejected reason=password_mismatch";
        return std::nullopt;
    }
    const auto user_id = id.get_oid().value.to_string();
    qCDebug(Infrastructure::Logging::auth) << "event=auth.password_check.completed identity_valid=" << !user_id.empty();
    return user_id.empty() ? std::nullopt : std::make_optional(user_id);
}

MongoDBDataManager::AccountCreationResult MongoDBDataManager::create_account(const QJsonObject& details) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.create_account");
    const auto email = details.value("email").toString().trimmed().toStdString();
    const auto password = details.value("password").toString().toStdString();
    qCDebug(Infrastructure::Logging::database) << "event=account.create.started";

    if (email.empty() || password.empty()) {
        qCWarning(Infrastructure::Logging::database) << "event=account.create.rejected reason=missing_fields";
        return AccountCreationResult::InvalidInput;
    }

    qCDebug(Infrastructure::Logging::database) << "event=account.duplicate_check.started";
    std::optional<mongocxx::collection> collection;
    try {
        collection = database()["Users"];
    } catch (const std::exception&) {
        qCWarning(Infrastructure::Logging::database)
            << "Could not initialize MongoDB. Check the connection configuration.";
        return AccountCreationResult::DatabaseError;
    }
    try {
        if (collection->find_one(make_document(kvp("UserEmail", email)))) {
            qCWarning(Infrastructure::Logging::database) << "event=account.create.rejected reason=duplicate_email";
            return AccountCreationResult::EmailExists;
        }
    } catch (const mongocxx::exception& e) {
        qCWarning(Infrastructure::Logging::database)
            << "event=account.duplicate_check.failed" << "code=" << e.code().value();
        return AccountCreationResult::DatabaseError;
    }

    qCDebug(Infrastructure::Logging::database) << "event=account.document.building";
    qCDebug(Infrastructure::Logging::auth) << "event=account.password_hash.started";
    const auto document = bsoncxx::builder::stream::document{}
                          << "UserEmail" << email << "UserPassword" << bcrypt::generateHash(password) << "FirstName"
                          << details.value("firstName").toString().trimmed().toStdString() << "LastName"
                          << details.value("lastName").toString().trimmed().toStdString() << "AccountSetupNeeded"
                          << true << bsoncxx::builder::stream::finalize;
    qCDebug(Infrastructure::Logging::auth) << "event=account.password_hash.completed";
    qCDebug(Infrastructure::Logging::database) << "event=account.insert.started";
    try {
        const auto result = collection->insert_one(document.view());
        if (result && result->result().inserted_count() == 1) {
            qCDebug(Infrastructure::Logging::database) << "event=account.insert.completed";
            return AccountCreationResult::Created;
        }
        qCWarning(Infrastructure::Logging::database) << "event=account.insert.failed reason=unacknowledged";
    } catch (const mongocxx::exception& e) {
        qCWarning(Infrastructure::Logging::database)
            << "event=account.insert.failed reason=database_exception" << "code=" << e.code().value();
        return AccountCreationResult::DatabaseError;
    }

    return AccountCreationResult::DatabaseError;
}

QJsonDocument MongoDBDataManager::get_account_businesses(const std::string& user_id) {
    QJsonObject result;
    for (const auto& value : list_resources("businesses", user_id).array()) {
        const auto business = value.toObject();
        result[business.value("_id").toString()] = business;
    }
    return QJsonDocument(result);
}

QJsonDocument MongoDBDataManager::list_resources(const std::string& resource,
                                                 const std::string& user_id,
                                                 const std::string& selected_business_id) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.list_resources");
    QJsonArray result;
    const auto owner = Infrastructure::Database::parse_object_id(user_id);
    const auto selected = Infrastructure::Database::parse_object_id(selected_business_id);
    if (!owner || (!selected_business_id.empty() && !selected) ||
        (resource != "businesses" && resource != "clients" && resource != "stocks"))
        return QJsonDocument(result);
    // Resolve ownership by the immutable MongoDB identity, never by a legacy display ID.
    const auto user = database()["Users"].find_one(make_document(kvp("_id", *owner)));
    if (!user)
        return QJsonDocument(result);
    bsoncxx::builder::basic::document filter;
    filter.append(kvp("UserID", *owner));
    if (selected)
        filter.append(kvp("_id", *selected));
    for (const auto& business : database()["Business"].find(filter.view())) {
        const auto business_id = business["_id"].get_oid().value;
        if (resource == "businesses") {
            QJsonObject item{{"_id", QString::fromStdString(business_id.to_string())}};
            // Legacy LogoPath values must never leave the database for rendering.
            for (const auto* field :
                 {"BusinessName", "ABN", "ACN", "Phone", "BusinessAddress", "Website", "LogoData", "InvoicePrefix"}) {
                const auto value = business[field];
                if (value && value.type() == bsoncxx::type::k_string)
                    item[field] = QString::fromUtf8(value.get_string().value);
            }
            item["Address"] = item.value("BusinessAddress");
            result.append(item);
            continue;
        }
        const auto children = make_document(kvp("BusinessID", business_id));
        for (const auto& view : database()[resource == "clients" ? "Clients" : "Stock"].find(children.view())) {
            QJsonObject item{{"_id", QString::fromStdString(view["_id"].get_oid().value.to_string())},
                             {"BusinessID", QString::fromStdString(business_id.to_string())}};
            const auto text = [&](const char* field) {
                const auto value = view[field];
                return value && value.type() == bsoncxx::type::k_string ? QString::fromUtf8(value.get_string().value)
                                                                        : QString{};
            };
            if (resource == "clients") {
                item["Name"] = text("ClientName");
                for (const auto* field : {"Phone", "Email", "Address"})
                    item[field] = text(field);
            } else {
                item["Name"] = text("Name");
                item["Price"] = view["StdPrice"].get_double().value;
                item["Quantity"] = view["Quantity"].get_int32().value;
                item["Unit"] = text("Unit Type");
                const auto margin = view["ProfitMargin"];
                if (margin && margin.type() == bsoncxx::type::k_double)
                    item["Margin"] = margin.get_double().value;
                QJsonArray keywords;
                const auto stored = view["ProductKeywords"];
                if (stored && stored.type() == bsoncxx::type::k_array)
                    for (const auto& word : stored.get_array().value)
                        if (word.type() == bsoncxx::type::k_string)
                            keywords.append(QString::fromUtf8(word.get_string().value));
                item["Keywords"] = keywords;
            }
            result.append(item);
        }
    }
    return QJsonDocument(result);
}

bool MongoDBDataManager::save_resource(const std::string& resource,
                                       QJsonObject data,
                                       const std::string& user_id,
                                       QString* saved_id) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.save_resource");
    try {
        const auto owner = Infrastructure::Database::parse_object_id(user_id);
        if (!owner || (resource != "business" && resource != "client" && resource != "stock"))
            return false;
        if (!database()["Users"].find_one(make_document(kvp("_id", *owner))))
            return false;
        std::string inserted_id;
        if (resource == "business") {
            QByteArray logo;
            const auto encoded = data.value("LogoData").toString().toLatin1();
            if (!encoded.isEmpty()) {
                QString error;
                if (encoded.size() > (BusinessLogo::maximum_bytes + 2) / 3 * 4 ||
                    !BusinessLogo::normalise(QByteArray::fromBase64(encoded), logo, error))
                    return false;
            }
            const auto doc = make_document(kvp("UserID", *owner),
                                           kvp("ABN", data.value("ABN").toString().toStdString()),
                                           kvp("Phone", data.value("Business phone").toString().toStdString()),
                                           kvp("BusinessName", data.value("Business name").toString().toStdString()),
                                           kvp("BusinessAddress", data.value("Address").toString().toStdString()),
                                           kvp("ACN", data.value("ACN").toString().toStdString()),
                                           kvp("Website", data.value("Website").toString().toStdString()),
                                           kvp("LogoData", logo.toBase64().toStdString()),
                                           kvp("ClientIDs", bsoncxx::builder::basic::array{}),
                                           kvp("StockIDs", bsoncxx::builder::basic::array{}));
            if (!insertDocument("Business", doc.view(), &inserted_id))
                return false;
            const auto user = findById("Users", user_id);
            if (!user)
                return false;
            const auto update =
                make_document(kvp("$addToSet", make_document(kvp("BusinessIDs", bsoncxx::oid(inserted_id)))));
            const auto result = database()["Users"].update_one(make_document(kvp("_id", *owner)), update.view());
            qCDebug(Infrastructure::Logging::database)
                << "event=database.business_link.completed acknowledged=" << static_cast<bool>(result)
                << "matched_count=" << (result ? result->matched_count() : 0);
            if (result && result->matched_count() == 1 && saved_id)
                *saved_id = QString::fromStdString(inserted_id);
            return result && result->matched_count() == 1;
        }

        const auto business_id =
            Infrastructure::Database::parse_object_id(data.value("BusinessID").toString().toStdString());
        if (!business_id)
            return false;
        const auto business = findOne("Business", make_document(kvp("_id", *business_id), kvp("UserID", *owner)));
        if (!business)
            return false;
        bsoncxx::builder::basic::document doc;
        if (resource == "client") {
            doc.append(kvp("BusinessID", *business_id));
            doc.append(kvp("ClientName", data.value("Name").toString().toStdString()));
            doc.append(kvp("Phone", data.value("Phone").toString().toStdString()));
            doc.append(kvp("Email", data.value("Email").toString().toStdString()));
            doc.append(kvp("Address", data.value("Address").toString().toStdString()));
        } else {
            doc.append(kvp("BusinessID", *business_id));
            doc.append(kvp("Name", data.value("Name").toString().toStdString()));
            doc.append(kvp("Quantity", data.value("Quantity").toInt()));
            doc.append(kvp("StdPrice", data.value("Price").toDouble()));
            doc.append(kvp("ProfitMargin", data.value("Margin").toDouble()));
            const auto unit = data.value("Unit").toString().trimmed();
            doc.append(kvp("Unit Type", unit.isEmpty() ? std::string("each") : unit.toStdString()));
            bsoncxx::builder::basic::array keywords;
            for (const auto& keyword : data.value("Keywords").toString().split(',', Qt::SkipEmptyParts))
                keywords.append(keyword.trimmed().toLower().toStdString());
            doc.append(kvp("ProductKeywords", keywords));
        }
        if (!insertDocument(resource == "client" ? "Clients" : "Stock", doc.view(), &inserted_id))
            return false;
        const std::string key = resource == "client" ? "ClientIDs" : "StockIDs";
        const auto value = bsoncxx::oid(inserted_id);
        const auto update = make_document(kvp("$addToSet", make_document(kvp(key, value))));
        const auto result = database()["Business"].update_one(
            make_document(kvp("_id", *business_id), kvp("UserID", *owner)), update.view());
        qCDebug(Infrastructure::Logging::database)
            << "event=database.resource_link.completed acknowledged=" << static_cast<bool>(result)
            << "matched_count=" << (result ? result->matched_count() : 0);
        if (result && result->matched_count() == 1 && saved_id)
            *saved_id = QString::fromStdString(inserted_id);
        return result && result->matched_count() == 1;
    } catch (const std::exception& error) {
        qCWarning(Infrastructure::Logging::database) << "event=resource.save.failed reason=exception";
        return false;
    }
}

void MongoDBDataManager::updateDoc(const std::string& collectionName,
                                   std::optional<bsoncxx::document::value> filterDoc,
                                   std::optional<bsoncxx::document::value> replacementDoc) {
    auto result = database()[collectionName].update_one(filterDoc->view(), replacementDoc->view());
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
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::database(),
                                                 "MongoDBDataManager.getCollection");
    try {
        return database()[collectionName];
    } catch (const mongocxx::exception& e) {
        qCWarning(Infrastructure::Logging::database) << "event=database.exception code=" << e.code().value();
        std::cerr << "collection probably does not exist" << std::endl;
        return std::nullopt;
    }
}
