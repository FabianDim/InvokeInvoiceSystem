#include "SetUserFromDB.h"
#include <bsoncxx/document/view_or_value.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
shared_ptr<User> SetUser::setUserOnLogin(const string& email) {
    MongoDBDataManager mongoDataManager;
	auto filter = bsoncxx::builder::stream::document{} 
    << "UserEmail" << email << bsoncxx::builder::stream::finalize;
    mongoDataManager.findOne("Users", filter);
    
    
    return shared_ptr<User>();
}
