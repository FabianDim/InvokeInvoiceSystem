#pragma once
#include "pch.h"

#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
namespace Infrastructure {
namespace Services {
class BusinessService {
  private:
    MongoDBDataManager& db_manager_;

  public:
    BusinessService(MongoDBDataManager& db_manager);
    ~BusinessService() = default;
    std::optional<std::string> fetchBusinessName(const std::string& businessID);
};
} // namespace Services
} // namespace Infrastructure