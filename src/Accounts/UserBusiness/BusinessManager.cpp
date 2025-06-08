#include "Accounts/UserBusiness/BusinessManager.h"
std::shared_ptr<BusinessRepository> BusinessManager::currentBusiness = nullptr;

void BusinessManager::setBusinessGlobally(const std::string bizID) {
	SetBusiness setBusiness;
	currentBusiness = setBusiness.setUpBusiness(bizID);
}

std::shared_ptr<BusinessRepository> BusinessManager::getBusiness() {
	if (currentBusiness == nullptr) {
		//std::cerr << "Business is not set." << std::endl;
		return nullptr;
	}
	return currentBusiness;
}
