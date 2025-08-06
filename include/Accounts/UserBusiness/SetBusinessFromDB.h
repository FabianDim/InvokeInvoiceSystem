#pragma once

#include "../../pch.h"

class BusinessRepository;

class SetBusiness {
	friend class BusinessMenu;
	friend class BusinessManager;
private:
	std::shared_ptr<BusinessRepository> setUpBusiness(const std::string businessID);
	std::string getBusinessName(const std::string businessID);
};