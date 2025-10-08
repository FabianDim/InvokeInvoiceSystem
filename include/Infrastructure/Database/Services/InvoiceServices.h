#pragma once
#include "pch.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class InvoiceServices {
  public:
    InvoiceServices(MongoDBDataManager& db_manager);
    ~InvoiceServices() = default;

    bool save_invoice(QJsonDocument& doc);

  private:
    MongoDBDataManager& db_manager_;
};
;