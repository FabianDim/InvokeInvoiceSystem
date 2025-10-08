#include "Infrastructure/Database/Services/AccountServices.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "Domain/Accounts/User.h"

AccountServices::AccountServices(MongoDBDataManager& db_manager,
                                 Invoke::Domain::Accounts::IAccountManager* account_manager)
    : db_manager(db_manager), account_manager_(account_manager) {}

bool AccountServices::validate_login(QJsonDocument& login_data) {
    QJsonObject obj = login_data.object();

    std::string email = obj.value("email").toString().toStdString();
    std::string password = obj.value("password").toString().toStdString();

    return db_manager.valid_password(password, email);
}

QJsonDocument AccountServices::get_account_businesses() {
    return db_manager.get_account_businesses(account_manager_->getAccount()->getMongoUserID());
}