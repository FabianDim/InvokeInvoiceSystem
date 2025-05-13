#include "MongoDBHandler.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

// Static instance initialization
mongocxx::instance MongoDBHandler::instance{};

MongoDBHandler::MongoDBHandler()
    : client(mongocxx::uri{std::getenv("MONGODB_URI") ? std::getenv("MONGODB_URI") : "mongodb://localhost:27017"})
{
}

mongocxx::database MongoDBHandler::getDatabase() const {
    return client["InvokeInvoiceSystem"];
}
