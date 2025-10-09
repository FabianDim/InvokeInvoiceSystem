#include "Infrastructure/Database/Services/InvoiceServices.h"
#include <QJsonObject>
InvoiceServices::InvoiceServices(MongoDBDataManager& db_manager) : db_manager_(db_manager) {}

bool InvoiceServices::save_invoice(QJsonDocument& doc) {
    QJsonObject obj = doc.object();
    try {
        db_manager_.get_account_businesses(obj.value("UserID").toString().toStdString());
    } catch (const std::exception& e) {
        return true;
    }
    return false;
}
