#include "Infrastructure/Database/Mongo/MongoDBHandler.h"
#include "Infrastructure/Logging/Logging.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <QDebug>
#include <QStringList>
#include <qtenvironmentvariables.h>

// Static instance initialization
mongocxx::instance MongoDBHandler::instance{};

namespace {
constexpr char kDefaultDatabaseName[] = "InvokeInvoiceSystem";
constexpr char kDefaultMongoUri[] = "mongodb://localhost:27017/InvokeInvoiceSystem";

std::string environmentVariableOrEmpty(const char* name) {
    const auto* value = std::getenv(name);
    return value ? value : "";
}

std::string resolveMongoUri() {
    const auto uri = environmentVariableOrEmpty("MONGODB_URI");
    return uri.empty() ? kDefaultMongoUri : uri;
}

std::string resolveDatabaseName(const mongocxx::uri& parsedUri) {
    const auto database = environmentVariableOrEmpty("MONGODB_DATABASE");
    if (!database.empty()) {
        return database;
    }

    const auto uriDatabase = parsedUri.database();
    return uriDatabase.empty() ? kDefaultDatabaseName : uriDatabase;
}

} // namespace

MongoDBHandler::MongoDBHandler()
    : uri(mongocxx::uri{resolveMongoUri()}), client(uri), databaseName(resolveDatabaseName(uri)) {
    qCDebug(Infrastructure::Logging::database) << "event=database.client.configured host_count=" << uri.hosts().size()
        << "credentials_present=" << !uri.username().empty();

    if (!uri.username().empty() && uri.password().empty()) {
        qWarning() << "[MongoDB] Username is present but password is empty. Check MONGODB_URI escaping.";
    }
}

mongocxx::database MongoDBHandler::getDatabase() const {
    return client[databaseName];
}
