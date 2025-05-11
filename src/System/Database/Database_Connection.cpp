#include "Database_Connection.h"

DatabaseConnection::DatabaseConnection() : client(mongocxx::uri(theKey)) {
}

mongocxx::database DatabaseConnection::getDatabase() {
	return client["InvokeInvoiceSystem"];
}
