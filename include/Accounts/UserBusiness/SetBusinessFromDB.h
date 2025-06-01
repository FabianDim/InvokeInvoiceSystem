#pragma once

#include "../../pch.h"
#include "Accounts/UserBusiness/BusinessRepository.h"

class SetBusiness {
	friend class BusinessMenu;
private:
	std::shared_ptr<BusinessRepository> setUpBusiness(const std::string businessID);
};