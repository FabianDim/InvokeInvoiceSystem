#pragma once
#include "pch.h"
#include "System/Database/MongoDBDataManager.h"

class BusinessRepository;

class BusinessManager{
public:
	static void setBusinessGlobally(const std::string bizID);
	static std::shared_ptr<BusinessRepository> getBusiness();
	//static std::shared_ptr<BusinessRepository> currentBusiness;
	bool chooseBusiness();
private:
	MongoDBDataManager dbManager;
	std::map<std::string, std::string> retrieveUsersBusinessIDs();
	bool validateUserBusiness(const std::string businessID);
	static std::shared_ptr<BusinessRepository> currentBusiness;
};
