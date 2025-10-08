#include "Infrastructure/Database/Services/InvoiceServices.h"
InvoiceServices::InvoiceServices(MongoDBDataManager& db_manager) : db_manager_(db_manager) {}

bool InvoiceServices::save_invoice(QJsonDocument& doc) {

    return false;
}
