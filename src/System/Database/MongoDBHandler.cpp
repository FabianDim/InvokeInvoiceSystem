#include "MongoDBHandler.h"
#include <cstdlib> 

mongocxx::instance MongoDBHandler::instance{};

MongoDBHandler::MongoDBHandler()
	: client{ mongocxx::uri{ std::getenv("INVOICE_DB_URI") } } {
}

mongocxx::database MongoDBHandler::getDatabase() const {
	return client["InvokeInvoiceSystem"];
}
