#include "Infrastructure/Database/Services/AccountServcies.h"
#include <QJsonObject>
#include <QJsonDocument>
AccountServices::AccountServices(MongoDBDataManager& db_manager) : db_manager(db_manager) {}

bool AccountServices::validate_login(QJsonDocument& login_data) {
    QJsonObject obj = login_data.object();

    std::string email = obj.value("email").toString().toStdString();
    std::string password = obj.value("password").toString().toStdString();

    return db_manager.valid_password(password, email);
}