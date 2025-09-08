#pragma once

#include "pch.h"

#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class AccountServices {
  public:
    AccountServices(MongoDBDataManager& db_manager);
    ~AccountServices() = default;
    bool validate_login(QJsonDocument& login_data);

  private:
    MongoDBDataManager& db_manager;
};
;