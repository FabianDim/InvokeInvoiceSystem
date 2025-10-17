#pragma once
#include "pch.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include "Domain/Invoices/Invoice.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <optional>
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
namespace Infrastructure::Services {
class InvoiceServices {
  public:
    InvoiceServices(MongoDBDataManager& db_manager);
    ~InvoiceServices() = default;

    bool save_invoice(QJsonDocument& doc);
    void add_business_to_invoice(const QJsonDocument doc);
    void begin_invoice_details(const QJsonDocument& doc);

  private:
    Invoice invoice_;
    MongoDBDataManager& db_manager_;
};
} // namespace Infrastructure::Services