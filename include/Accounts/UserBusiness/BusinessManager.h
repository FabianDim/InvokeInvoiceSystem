#pragma once
#include "pch.h"
#include "BusinessRepository.h"
#include "SetBusinessFromDB.h"
class BusinessManager{
public:
	static void setBusinessGlobally(const std::string bizID);
	static std::shared_ptr<BusinessRepository> getBusiness();
	static std::shared_ptr<BusinessRepository> currentBusiness;
};