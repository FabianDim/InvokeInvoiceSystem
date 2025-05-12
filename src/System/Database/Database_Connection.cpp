#include "Database_Connection.h"
#include <cstdlib> 


DatabaseConnection::DatabaseConnection()
	: client{ mongocxx::uri{ std::getenv("INVOICE_DB_URI") } } {
}

mongocxx::database DatabaseConnection::getDatabase() {
	return client["InvokeInvoiceSystem"];
}
