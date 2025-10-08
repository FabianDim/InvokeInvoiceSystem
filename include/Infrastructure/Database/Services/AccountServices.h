#pragma once

#include "pch.h"

#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include <qjsondocument.h>
#include "Domain/Accounts/Interfaces/IAccountManager.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class AccountServices {
  public:
    AccountServices(MongoDBDataManager& db_manager, Invoke::Domain::Accounts::IAccountManager* account_manager);
    ~AccountServices() = default;
    bool validate_login(QJsonDocument& login_data);
    QJsonDocument get_account_businesses();

  private:
    MongoDBDataManager& db_manager;
    Invoke::Domain::Accounts::IAccountManager* account_manager_;
};
;