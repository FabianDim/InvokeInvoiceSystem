#include "BusinessService.h"
#include <QJsonObject>
#include <QJsonDocument>

Infrastructure::Services::BusinessService::BusinessService(MongoDBDataManager& db_manager, BusinessRepository biz_repo)
    : biz_repo_(db_manager), db_manager_(db_manager) {}

QJsonDocument Infrastructure::Services::BusinessService::create_business_object(const QJsonObject obj) {
    QJsonDocument doc(obj);
    return doc;
}
