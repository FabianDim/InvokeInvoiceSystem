#pragma once

#include "pch.h"
#include "BusinessRepository.h"

class SetBusiness {
	friend class InvoiceMenu;
private:
	std::shared_ptr<BusinessRepository> setUpBusiness(const std::string businessID);
};