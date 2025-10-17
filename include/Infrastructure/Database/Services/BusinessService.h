#pragma once
#include "pch.h"
#include "Application/Business/BusinessRepository.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include "Application/Business/SetBusinessFromDB.h"
#include <optional>
namespace Infrastructure {
namespace Services {
class BusinessService {
  private:
    MongoDBDataManager& db_manager_;
    BusinessRepository biz_repo_;

  public:
    BusinessService(MongoDBDataManager& db_manager, BusinessRepository biz_repo);
    ~BusinessService() = default;
    // std::optional<std::string> fetchBusinessName(const std::string& businessID);
    QJsonDocument create_business_object(const QJsonObject obj);
};
} // namespace Services
} // namespace Infrastructure