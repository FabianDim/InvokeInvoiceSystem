#include "BusinessRepository.h"


BusinessRepository::BusinessRepository(std::string bizID) : currentUser(AccountManager::currentUser) {
	this->bizID = bizID;
}

//getters and setters;

