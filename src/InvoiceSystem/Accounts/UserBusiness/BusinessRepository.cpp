#include "Accounts/UserBusiness/BusinessRepository.h"
#include "Accounts/AccountManager.h"
#include "Accounts/User.h"
#include "System/Database/MongoDBDataManager.h"

BusinessRepository::BusinessRepository(MongoDBDataManager& dbManager)
    : currentUser(AccountManager::currentUser), dbManager(dbManager) {
    if (currentUser) {
        currentUserID = currentUser->getMongoUserID();
    }
}

//getters and setters;

